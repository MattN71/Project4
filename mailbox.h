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
	bool isGenuine;
};

//Function Prototypes
void mailbox(std::string input, std::string output);
void grow(message* &myArray, int &size);
unsigned int loadMessage(message* messageArray, int &messageArraySize, int &arrayIndex, std::string fileName);
unsigned int addMessage(message* messageArray, int &messageArraySize, int &arrayIndex, std::ifstream &in, std::string &temp);
void displayInbox(std::ofstream &out, message* &messageArray, int arrayIndex, bool entireInbox, int whichOne = -1);


#endif /* MAILBOX_H */