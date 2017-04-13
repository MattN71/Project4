#ifndef MAILBOX_H
#define MAILBOX_H

#include <string>

//Struct to store the parts of each message
struct message { 
	std::string from;
	std::string date;
	std::string to;
	std::string subject;
	std::string message;
	unsigned int id;
};

//Function Prototypes
void mailbox(std::string input, std::string output);
void grow(message* &myArray, int &size);
unsigned int loadMessage(message* &messageArray, int &messageArraySize, int &arrayIndex, std::string fileName);
void addMessage(message* &messageArray, int &messageArraySize, int &arrayIndex, std::ifstream &in, std::string &temp);
void displayInbox(std::ofstream &out, message* &messageArray, int arrayIndex, bool entireInbox, int whichOne = -1);
void saveInbox(std::ofstream &out, message* &messageArray, int arrayIndex);
bool removeMessage(std::ofstream &out, message* &messageArray, int &arrayIndex, int messageToRemove);
bool checkIfHacked(message* &goodArray, int &goodIndex, message* &hackedArray, int &hackedIndex, int &hackedSize, int messageNum, std::ofstream &out);

#endif /* MAILBOX_H */