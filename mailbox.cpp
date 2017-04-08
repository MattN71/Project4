#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "mailbox.h"
#include "sha256.h"

void mailbox(std::string input, std::string output) {
	std::ifstream command(input); //Open input file stream
	std::ofstream out(output); //Open output file stream

	int goodSize = 10; //Size of good message array
	message* goodMsgPtr = new message[goodSize]; //Pointer to good message array
	int goodIndex = 0; //Index for location in array
	
	
	int hackedSize = 10; //Size of hacked message array
	message *hackedMsgPtr = new message[hackedSize]; //Pointer to hacked message array
	int hackedIndex = 0; //Index for location in array
	
	
	std::string temp;
	command >> temp;
	while ( !command.fail() ) {
		if (temp == "load") {
			command >> temp; //Store filename into temp
			out << "Command: load " << temp << std::endl;
			unsigned int num = loadMessage(goodMsgPtr, goodSize, goodIndex, temp); //Load messages into good inbox
			out << "\tMessages: " << num << std::endl;
		} else if (temp == "add") {
			command >> temp; //Store "From" into temp
			out << "Command: add " << std::endl;
			unsigned int result = addMessage(goodMsgPtr, goodSize, goodIndex, command, temp); //Add message to good Inbox		
			if (goodMsgPtr[goodIndex].id != result) { //If message is hacked
				//Move message to bad inbox
			}
			out << "\tMessage added" << std::endl;
		} else if (temp == "show") {
			getline(command, temp);
			std::istringstream lineS(temp);
			std::string inbox;
			int num = 0;
			lineS >> inbox;
			lineS >> num;	
			out << "Command: show " << inbox;
			if (lineS.fail()) {
				out << std::endl;
			} else {
				out << " " << num << std::endl;
			}
			if (inbox == "inbox") {
				displayInbox(out, goodMsgPtr, goodIndex, lineS.fail(), num);
			} else if (inbox == "hacked") {
				displayInbox(out, hackedMsgPtr, hackedIndex, lineS.fail(), num);
			}
		} else if (temp == "remove") {
			
		} else if (temp == "save") {
			std::string whichOne;
			std::string outFileName;
			command >> whichOne;
			command >> outFileName;
			std::ofstream mboxOut(outFileName);
			if (whichOne == "inbox") {	
				saveInbox(mboxOut, goodMsgPtr, goodIndex);
			} else {
				saveInbox(mboxOut, hackedMsgPtr, hackedIndex);
			}
			mboxOut.close();
		} else {
			std::cout << "Unknown Command." << std::endl;
		}
		command >> temp;
	}	
	command.close();
	out.close();
}


//Function to double size of array and reallocate memory for array of message struct
void grow(message* &myArray, int &size) {
	//std::cout << "Resizing array from " << size << " to " << size*2 << std::endl;
	message *temp = myArray; //make a temporary pointer to the old array 
	myArray = new message[size*2]; //make a new larger array 	
	for (int i = 0; i < (size); i++) { //copy the element from the old full array into the new larger array 
		myArray[i] = temp[i]; 
	}
	delete [] temp; //release the old memory
	temp = nullptr; //null it out 
	size *= 2; //double the size 
	//std::cout << "Resizing successful" << std::endl;
}


//This function will load the messages from the given file into the given array. 
//If needed, it will call grow to increase the size of the array
unsigned int loadMessage(message* &messageArray, int &messageArraySize, int &arrayIndex, std::string fileName) {
	std::ifstream in(fileName); //Open new file stream
	std::string temp; //Temporary variable for extraction
	unsigned int count = 0;
	in >> temp; //Prime - "From" 
	while ( !in.fail() ) { //Test	
		addMessage(messageArray, messageArraySize, arrayIndex, in, temp); //Add message, ignore signature for this function
		in >> temp; ///Re-prime - "From"
		count++;
	}
	in.close(); //Close file stream
	return count;
}


//This function adds one message from the given input file stream to the array passed to the function. It also increments the array index
//and increases the size of the array if necessary. 
unsigned int addMessage(message* &messageArray, int &messageArraySize, int &arrayIndex, std::ifstream &in, std::string &temp) {

	if (arrayIndex == messageArraySize) { //If array is now full, increase size
		//std::cout << "Doubling array in addMessage function" << std::endl;
		grow(messageArray, messageArraySize); //Double size of array
	}
	
	//getline(in, messageArray[arrayIndex].from); //Read in "from" field
	in >> messageArray[arrayIndex].from;
	std::cout << "From field loaded: " << std::endl;
	
	in >> temp; //Read in "Date"
	getline(in, messageArray[arrayIndex].date);
	std::cout << "Date field loaded: " << std::endl;

	in >> temp; //Read in "To"
	getline(in, messageArray[arrayIndex].to);
	std::cout << "To field loaded: " << std::endl;
	
	in >> temp; //Read in "Subject"
	getline(in, messageArray[arrayIndex].subject);
	std::cout << "Subject field loaded: " << std::endl;

	getline(in, temp); //Read in "Message"
	getline(in, messageArray[arrayIndex].message);		
	std::cout << "Message field loaded: " << std::endl;
		
	in >> temp; //Read in "id xxxxx"
	in >> messageArray[arrayIndex].id; //Read in id number
	std::cout << "ID field loaded: " << std::endl;
		
	std::string rawString = messageArray[arrayIndex].from +  //Add fields together to form string to hash
							messageArray[arrayIndex].date + 
							messageArray[arrayIndex].to + 
							messageArray[arrayIndex].subject + 
							messageArray[arrayIndex].message;						 
	std::string hashedString = sha256(rawString); //Hash string using sha256
	unsigned int signature = sign(hashedString, messageArray[arrayIndex].from); //Calculate signature for message.
	//std::cout << "Calculated Signature: " << signature << std::endl << std::endl; //Display message signature
	
	arrayIndex++; //Increment index in array
	
	std::cout << "Array index now = " << arrayIndex << std::endl;
	return signature; //Return message signature
}

void displayInbox(std::ofstream &out, message* &messageArray, int arrayIndex, bool entireInbox, int whichOne) {
	if (entireInbox == true) { //If printing entire inbox
		for (int i = 0; i < arrayIndex; i++) {
			out << "From: " << messageArray[i].from << std::endl;
			out << "Date: " << messageArray[i].date << std::endl;
			out << "To: " << messageArray[i].to << std::endl;
			out << "Subject: " << messageArray[i].subject << std::endl;
			out << "Message: " << messageArray[i].message << std::endl;
			out << "ID: " << messageArray[i].id << std::endl;
		}
	} else { //If printing just a single message
		if (whichOne < arrayIndex) {
			out << "From: " << messageArray[whichOne].from << std::endl;
			out << "Date: " << messageArray[whichOne].date << std::endl;
			out << "To: " << messageArray[whichOne].to << std::endl;
			out << "Subject: " << messageArray[whichOne].subject << std::endl;
			out << "Message: " << messageArray[whichOne].message << std::endl;
			out << "ID: " << messageArray[whichOne].id << std::endl;
		} else {
			out << "\tInvalid Message Number." << std::endl;
		}
	}
}


void removeMessage(std::ofstream &out, message* &messageArray, int &arrayIndex, int messageToRemove) {
	if (messageToRemove >= arrayIndex) {
		out << "\tInvalid message number." << std::endl;
		return;
	}

	arrayIndex -= 1;
	for (int i = messageToRemove; i < (arrayIndex - 1); i++) { //Shift all messages down
		messageArray[i] = messageArray[i+1];
	}
}


void saveInbox(std::ofstream &out, message* &messageArray, int arrayIndex) {
	for (int i = 0; i < arrayIndex; i++) {
		out << "From " << messageArray[i].from << std::endl;
		out << "Date " << messageArray[i].date << std::endl;
		out << "To " << messageArray[i].to << std::endl;
		out << "Subject " << messageArray[i].subject << std::endl;
		out << "Message " << std::endl << messageArray[i].message << std::endl;
		out << "id " << messageArray[i].id << std::endl;
	}
}