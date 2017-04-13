#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "mailbox.h"
#include "sha256.h"

/* 
*  Purpose for program
*  This program will read in a command file, and process 5 different commands
*  1. Load will add messages from a mailbox file into the inbox, which is array of structs
*  2. Add will check a message's integrity, and add it either to the inbox, or an array of hacked messages.
*  3. Show will print out all messages in whichever inbox is specified
*  4. Remove will remove a given message from the specified mailbox
*  5. Save will save the current inbox to a mbox file
*  Each array of structs is dynamically allocated, and will double in size when the limit is reached.
* 
*  Programmer: Matt Nicklas
*  Date: April 5, 2017
*
*  Honor Code: I have neither given nor received any unauthorized assistance with this program.
*/

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
	command >> temp; //Prime
	while ( !command.fail() ) {
		if (temp == "load") { //Load command
			command >> temp; //Store filename into temp
			out << "Command: load " << temp << std::endl;
			unsigned int num = loadMessage(goodMsgPtr, goodSize, goodIndex, temp); //Load messages into good inbox
			out << "\tMessages: " << num << std::endl; //Display number of messages loaded
		} else if (temp == "add") {
			command >> temp; //Store "From" into temp
			out << "Command: add " << std::endl;
			addMessage(goodMsgPtr, goodSize, goodIndex, command, temp); //Add message to good Inbox		
			bool isHacked = checkIfHacked(goodMsgPtr, goodIndex, hackedMsgPtr, hackedIndex, hackedSize, (goodIndex - 1), out); //check authenticity
			if (isHacked) //If it as hacked
				out << "\tMessage was not authentic.  Sent to hacked list." << std::endl;
			else	//If it was not
				out << "\tMessage added" << std::endl;
		} else if (temp == "show") { //Show command
			getline(command, temp);
			std::istringstream lineS(temp);
			std::string inbox;
			int num = 0; //Message to show
			lineS >> inbox; //which inbox
			lineS >> num; //which message
			out << "Command: show " << inbox;
			if (lineS.fail()) { //If it failed getting a number
				out << std::endl;
			} else {
				out << " " << num << std::endl;
			}
			if (inbox == "inbox") {
				displayInbox(out, goodMsgPtr, goodIndex, lineS.fail(), num); //Display inbox
			} else if (inbox == "hacked") {
				displayInbox(out, hackedMsgPtr, hackedIndex, lineS.fail(), num); //display hacked inbox
				//std::cout << "Added to hacked inbox" << std::endl;
				//std::cout << "Hacked Index: " << hackedIndex << std::endl;
			}
		} else if (temp == "remove") { //Remove command
			std::string mailbox; //inbox or hacked inbox
			int num = 0; //message to remove
			command >> mailbox; //Load variables
			command >> num;
			out << "Command: remove " << mailbox << " " << num << std::endl;
			if (mailbox == "inbox") { 
				bool success = removeMessage(out, goodMsgPtr, goodIndex, num); //Remove message from inbox
				if (success)
					out << "\tMessage removed" << std::endl;
			} else {
				bool success = removeMessage(out, hackedMsgPtr, hackedIndex, num); //Remove message from hacked inbox
				if (success)
					out << "\tMessage removed" << std::endl;
			}
		} else if (temp == "save") { //Save message
			std::string whichOne; //Which inbox
			std::string outFileName; //Output file name
			command >> whichOne;
			command >> outFileName;
			out << "Command: save " << whichOne << " " << outFileName << std::endl; 
			std::ofstream mboxOut(outFileName); //open file stream for new file
			if (whichOne == "inbox") {	
				saveInbox(mboxOut, goodMsgPtr, goodIndex); //save inbox to file
				out << "\t" << goodIndex << " Messages saved" << std::endl;
			} else {
				saveInbox(mboxOut, hackedMsgPtr, hackedIndex); //save hacked inbox to file
				out << "\t" << hackedIndex << " Messages saved" << std::endl;
			}
			mboxOut.close(); //close new file stream
		} else {
			std::cout << "Unknown Command." << std::endl; //Error reading commands
		}
		command >> temp; //Re-prime
	}	
	command.close(); //Close file streams
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
void addMessage(message* &messageArray, int &messageArraySize, int &arrayIndex, std::ifstream &in, std::string &temp) {

	if (arrayIndex == messageArraySize) { //If array is now full, increase size
		//std::cout << "Doubling array in addMessage function" << std::endl;
		grow(messageArray, messageArraySize); //Double size of array
	}
	
	//getline(in, messageArray[arrayIndex].from); //Read in "from" field
	in >> messageArray[arrayIndex].from;
	
	in >> temp; //Read in "Date"
	in.ignore(1);
	getline(in, messageArray[arrayIndex].date);

	in >> temp; //Read in "To"
	in.ignore(1);
	getline(in, messageArray[arrayIndex].to);

	in >> temp; //Read in "Subject"
	in.ignore(1);
	getline(in, messageArray[arrayIndex].subject);

	getline(in, temp); //Read in "Message"
	getline(in, messageArray[arrayIndex].message);		
		
	in >> temp; //Read in "id xxxxx"
	in >> messageArray[arrayIndex].id; //Read in id number
	
	arrayIndex++; //Increment index in array
	
}

//This function will either display a single message, or the entire mailbox, depending on the input.
//The last parameter is optional if the bool is true
void displayInbox(std::ofstream &out, message* &messageArray, int arrayIndex, bool entireInbox, int whichOne) {
	if (arrayIndex == 0) { //Immediately return if array if empty
		return;
	}
	
	if (entireInbox == true) { //If printing entire inbox
		for (int i = 0; i < arrayIndex; i++) {
			out << "From " << messageArray[i].from << std::endl; //loop through entire array and print out each message field
			out << "Date " << messageArray[i].date << std::endl;
			out << "To " << messageArray[i].to << std::endl;
			out << "Subject " << messageArray[i].subject << std::endl;
			out << "Message " << std::endl << messageArray[i].message << std::endl;
			out << "ID " << messageArray[i].id << std::endl << std::endl;
		}
	} else { //If printing just a single message
		if (whichOne < arrayIndex && whichOne >= 0) {
			out << "From " << messageArray[whichOne].from << std::endl; //print a single message 
			out << "Date " << messageArray[whichOne].date << std::endl;
			out << "To " << messageArray[whichOne].to << std::endl;
			out << "Subject " << messageArray[whichOne].subject << std::endl;
			out << "Message " << std::endl << messageArray[whichOne].message << std::endl;
			out << "ID " << messageArray[whichOne].id << std::endl;
		} else {
			out << "\tMessage " << whichOne << " not a valid message number" << std::endl; //Print out error is message number is invalid
		}
	}
}

//This function will check if a message number is valid, and then remove it from the mailbox if so. 
//Then it will shift down the messages above the one removed. It will return whether it was removed or not
bool removeMessage(std::ofstream &out, message* &messageArray, int &arrayIndex, int messageToRemove) {
	if (messageToRemove >= arrayIndex || messageToRemove < 0) {
		out << "\tMessage not removed" << std::endl;
		out << "\tMessage " << messageToRemove << " not a valid message number" << std::endl;
		return false;
	}

	for (int i = messageToRemove; i < (arrayIndex - 1); i++) { //Shift all messages down
		messageArray[i] = messageArray[i+1];
	}
	arrayIndex -= 1; //Reduce array index by 1
	return true; //Return success
}

//This function will take a mailbox array and output it to a mbox file, which could then be loaded using the loadMessage function.
void saveInbox(std::ofstream &out, message* &messageArray, int arrayIndex) {
	for (int i = 0; i < arrayIndex; i++) { //loop through array and save to a new file
		out << "From " << messageArray[i].from << std::endl;
		out << "Date " << messageArray[i].date << std::endl;
		out << "To " << messageArray[i].to << std::endl;
		out << "Subject " << messageArray[i].subject << std::endl;
		out << "Message " << std::endl << messageArray[i].message << std::endl;
		out << "ID " << messageArray[i].id << std::endl << std::endl;
	}
}

//This function will check to see if a message is hacked, and if so, will move it from the inbox to the hacked inbox.
bool checkIfHacked(message* &goodArray, int &goodIndex, message* &hackedArray, int &hackedIndex, int &hackedSize, int messageNum, std::ofstream &out) {
	std::string rawString = goodArray[messageNum].from +  //Add fields together to form string to hash
							goodArray[messageNum].date + 
							goodArray[messageNum].to + 
							goodArray[messageNum].subject + 
							goodArray[messageNum].message;				
	std::string hashedString = sha256(rawString); //Hash string using sha256
	unsigned int signature = sign(hashedString, (goodArray[messageNum].from)); //Calculate signature for message.
	
	if (signature == goodArray[messageNum].id) { //If message is genuine
		return false; //Return, do nothing
	} else { //If message is hacked
		
		if (hackedIndex == hackedSize) { //If array is now full, increase size
			grow(hackedArray, hackedSize); //Double size of array
		}
	
		hackedArray[hackedIndex].from = goodArray[messageNum].from; //Copy message from inbox to hacked inbox
		hackedArray[hackedIndex].date = goodArray[messageNum].date;
		hackedArray[hackedIndex].to = goodArray[messageNum].to;
		hackedArray[hackedIndex].subject = goodArray[messageNum].subject;
		hackedArray[hackedIndex].message = goodArray[messageNum].message;
		hackedArray[hackedIndex].id = goodArray[messageNum].id;
		hackedIndex++; //Increment hacked inbox size by 1 
		removeMessage(out, goodArray, goodIndex, messageNum); //Remove message from inbox
		return true; //Return true
	}
}
