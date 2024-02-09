import sys
from PyQt6.QtWidgets import QApplication
from PyQt6.QtWebEngineWidgets import QWebEngineView
from PyQt6.QtCore import QUrl
import plotly.graph_objects as go
import dash
import dash_core_components as dcc
import dash_html_components as html


class Node:
    def __init__(self, index, children, properties):
        self.index = index
        self.children = children
        self.properties = properties
        self.width = 1

class Tree:
    def __init__(self) -> None:
        self.nodes = {}
    
    def add_node(self, node):
        self.nodes[node.index] = node
    
    def get_children(self, index):
        return [self.nodes[i] for i in self.nodes[index].children]

    def calculate_node_widths(self):
        def calculate_width(node):
            if not node.children:
                node.width = 1
            else:
                children = self.get_children(node.index)
                #calculate width of all children
                for child in children:
                    calculate_width(child)
                
                node.width = max(sum([child.width for child in children]), len(node.children))
        
        calculate_width(self.nodes[0])
        
    def plot_tree(self):
        eps = 0.05
        self.calculate_node_widths()
        positions = {}
        sizes = {}
        node_queue = [(0, 0, 0, 1)]
        while node_queue:
            index, depth, left, right = node_queue.pop(0)
            if depth > 3:  # Skip nodes with depth > 4
                continue
            node = self.nodes[index]
            x = (left + right) / 2
            y = -depth
            positions[index] = (x, y)
            sizes[index] = (right - left)/8 * 1000
            children = self.get_children(index)
            #distribute space proportionally to widths
            total_width = sum([child.width for child in children])
            left_child = left + eps * (right - left)
            for child in children:
                right_child = left_child + (child.width / total_width) * (right - left)*(1 - 2*eps)
                node_queue.append((child.index, depth + 1, left_child, right_child))
                left_child = right_child

        # Create edge trace
        edge_trace = go.Scatter(
            x=[pos for node in self.nodes.values() if node.index in positions for child in node.children if child in positions for pos in (positions[node.index][0], positions[child][0], None)],
            y=[pos for node in self.nodes.values() if node.index in positions for child in node.children if child in positions for pos in (positions[node.index][1], positions[child][1], None)],
            mode='lines'
        )

        # Create node trace
        node_trace = go.Scatter(
            x=[pos[0] for index, pos in positions.items()],
            y=[pos[1] for index, pos in positions.items()],
            mode='markers',
            text=[f"move: {self.nodes[index].properties.get('move', 'root')} score: {self.nodes[index].properties.get('score', '0')}" for index in positions.keys()],
            marker=dict(size=[sizes[index] for index in positions.keys()])
        )

        # Create figure
        fig = go.Figure(data=[edge_trace, node_trace])

        return fig
def parse_log(log_file):
    #create a graph
    T = Tree()
    with open(log_file, 'r', errors='ignore') as f:
        lines = f.readlines()
        print(len(lines))
        for lines in lines:
            fields = lines.split(";")
            index = int(fields[0])
            children_size = int(fields[1])
            properties_size = int(fields[2])

            children_string = fields[3].split(",")
            children = [int(children_string[i]) for i in range(children_size)]

            properties_string = fields[4].split(",")
            properties = {x[0]: x[1] for x in [properties_string[i].split(":") for i in range(properties_size)]}
            T.add_node(Node(index, children, properties))
    return T

# Create a Dash application
app = dash.Dash(__name__)

T = parse_log("search_log.txt")
print("parse log done")
figure1 = T.plot_tree()

T2 = parse_log("search_log2.txt")
print("parse log done")
figure2 = T2.plot_tree()

# Add the plots to the Dash application
app.layout = html.Div([
    dcc.Graph(figure=figure1),
    dcc.Graph(figure=figure2)
])

# Run the Dash application on a local server
app.run_server(debug=True)

# Create a Qt application
qt_app = QApplication(sys.argv)

# Create a QWebEngineView
web_view = QWebEngineView()

# Load the local server's URL into the QWebEngineView
web_view.load(QUrl("http://127.0.0.1:8050/"))

# Show the QWebEngineView
web_view.show()

# Run the Qt application
sys.exit(qt_app.exec())