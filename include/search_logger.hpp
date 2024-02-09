#ifndef SEARCH_LOGGER_HPP	
#define SEARCH_LOGGER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "omp.h"
class SearchNode{
    public:
        SearchNode(int indexInTree, std::vector<std::pair<std::string, std::string>> properties){
            this->indexInTree = indexInTree;
            this->properties = properties;
            this->childrenIndices = {};

            
        };
        

        void addChild(int childIndex){
            childrenIndices.push_back(childIndex);
        };
        
        void addProperty(std::pair<std::string, std::string> property){
            #pragma omp critical
            properties.push_back(property);
        }

        int getChild(int index){
            return childrenIndices[index];
        }


        int getNbChildren(){
            return childrenIndices.size();
        }

        std::string getProperty(std::string key){
            for (int i = 0; i < properties.size(); i++){
                if (properties[i].first == key){
                    return properties[i].second;
                }
            }
            return "";
        }
        //to string
        std::string toString(){
            std::string result = "";

            result += std::to_string(indexInTree) + ";";
            result += std::to_string(childrenIndices.size()) + ";";
            result += std::to_string(properties.size()) + ";";
            for(int i = 0; i < childrenIndices.size(); i++){
                result += std::to_string(childrenIndices[i]) + ",";
            }
            result += ";";
            for (int i = 0; i < properties.size(); i++){
                result += properties[i].first + ":" + properties[i].second + ",";
            }

            return result;
        }

    private:
        int indexInTree;
        std::vector<std::pair<std::string, std::string>> properties;       
        std::vector<int> childrenIndices;

};
class SearchLogger {
    public:
        SearchLogger(std::string filename){
            this->filename = filename;
            std::vector<std::pair<std::string, std::string>> properties;
            createSearchNode(properties);
        };

        int addChild(int index, std::vector<std::pair<std::string, std::string>> properties){
            
            int newIndex = createSearchNode(properties);
            
            nodes[index]->addChild(newIndex);
            return newIndex;
        };

        void write(){
            
            std::ofstream file;
            file.open(filename);
            
            for (int i = 0; i < nodes.size(); i++){
                file << nodes[i]->toString() << std::endl;
            }
            file.close();
        }

        void addPropertyToLastChild(int parentIndex, std::pair<std::string, std::string> property){
            nodes[nodes[parentIndex]->getChild(nodes[parentIndex]->getNbChildren() - 1)]->addProperty(property);
        }

        void addPropertyToChildWithMatchingFen(int parentIndex, std::string fen, std::pair<std::string, std::string> property){
            for (int i = 0; i < nodes[parentIndex]->getNbChildren(); i++){
                if (nodes[nodes[parentIndex]->getChild(i)]->getProperty("fen") == fen){
                    nodes[nodes[parentIndex]->getChild(i)]->addProperty(property);
                    return;
                }
            }
        }

        void eraseLog(){
            //delete all nodes and root
            nodes.clear();


            //create new root
            std::vector<std::pair<std::string, std::string>> properties;
            createSearchNode(properties);

        }
    
    private: 
        std::string filename;
        std::vector<SearchNode*> nodes;
        int createSearchNode(std::vector<std::pair<std::string, std::string>> properties){
            int newIndex;
            #pragma omp critical
            {
                newIndex = nodes.size();
                SearchNode* newNode = new SearchNode(newIndex, properties);
            
                nodes.push_back(newNode);
            }
            return newIndex;
        }
};
#endif // SEARCH_LOGGER_HPP