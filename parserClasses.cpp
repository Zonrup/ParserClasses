//Use only the following libraries:
#include "parserClasses.h"
#include <string>


/*
All code written by:
Jason Liu 301224956
Daryl Seah 301222124



*/
//Complete the implementation of the following member functions:

//****TokenList class function definitions******

//Creates a new token for the string input, str
//Appends this new token to the TokenList
//On return from the function, it will be the last token in the list
void TokenList::append(const string &str) {
	if (str.length() == 0) {
		return;
	}
	Token *d = new Token(str);
	this->append(d);
	return;
}

//Appends the token to the TokenList if not null
//On return from the function, it will be the last token in the list
void TokenList::append(Token *token) {
	if (!token)
		return;
	token->setPrev(getLast());
	token->setNext(NULL);
	if (tail) //if tail is not null, then there is at least one element in the list.
		tail->setNext(token); //sets current final to point to the new one
	else
		head = token; //if tail is null, that means head is also null. head needs to point to token.
	tail = token; //at the end, tail has to point to the newly appended token.
	return;
}

//Removes the token from the linked list if it is not null
//Deletes the token
//On return from function, head, tail and the prev and next Tokens (in relation to the provided token) may be modified.
void TokenList::deleteToken(Token *token) {
	if (!token)
		return;
	if (token == head && token == tail) {
		head = NULL;
		tail = NULL;
	}
	else if (token == head) {
		token->next->prev = NULL;
		head = token->next;
	}
	else if (token == tail) {
		token->prev->next = NULL; //makes the previous one point to null
		tail = token->prev; //moves tail to the previous one
	}
	else {
		token->next->prev = token->prev; //makes the next one point behind it
		token->prev->next = token->next; //makes the previous one point ahead of it
	}
	delete token; //unless token was null, token will be deleted in the end.
	return;
}


//****Tokenizer class function definitions******

//Computes a new tokenLength for the next token
//Modifies: size_t tokenLength, and bool complete
//(Optionally): may modify offset
//Does NOT modify any other member variable of Tokenizer
void Tokenizer::prepareNextToken(){
	tokenLength = 0;
	size_t len = str->length();
	if (!complete && !processingInlineComment && !processingBlockComment)
		trimwhitespace();

//if the character is the end of the line
	if (offset == len)  {
		complete = true;
		return;
	}

//if complete is true for some reason
	if (complete)
		return;

//if the first character of the token is a number,
	if (isnumber((char)str->at(offset))) {
		processingFloat = true;
	}
	else if (str->at(offset) == '.') {
		if (!(offset+1 == len)) {
			if (isnumber((char)str->at(offset+1))){
				processingFloat = true;
			}
		}
	}
	//boolean true processing
	if (processingInlineComment) {
		while (offset < len) {
			offset++;
			tokenLength++;
			if (offset == len) {
				complete = true;
				break;
			}
		}
		return;
	}
	else if (processingBlockComment) { //this is really convoluted and stupid. can be refactored
		if (offset == len)
		{
			complete = true;
			return;
		}
		if (str->at(offset) == '*') {//if the next characters are */, stop the block comment. token is length 2.
			if (offset+1 == len) {//if the line ends in a *, add it to the current token.
				offset++;
				tokenLength++;
				complete = true;
				return;
			}
			else if (str->at(offset+1) == '/') {
				offset += 2;
				tokenLength = 2;
				return;
			}
		}
		else {//otherwise, continue adding characters to the token until */ is found. after that, stop adding characters.
			while(1) {
				if (str->at(offset) != '*') { //no * found, just keep reading.
					offset++;
					tokenLength++;
					if (offset == len) {//if the character is the end of the line
						complete = true;
						break;
					}
				} //note that processingBlockComment is never set to false, unless the end of the block comment is found.
				else {//if there is a *
					if (offset+1 == len) {//if it's the last character in the line, just add it to the current token and return.
						offset++;
						tokenLength++;
						complete = true;
						break;
					}
					else {//if it isn't the last character,
						if (str->at(offset+1) == '/') { //if the next is a /, the */ condition wil execute in the top-level if.
							//ignore the * and the /, and everything before it was the block comment.
							break;
						}
						else { //otherwise, you just add it to the token. It won't be the end of the line.
							offset++;
							tokenLength++;
						}
					}
				}
			}
			return;
		}
	}
	else if (processingFloat) { //this probably can be refactored too
		while(isnumber((char)str->at(offset)) || str->at(offset) == 'e' || str->at(offset) == '.' || str->at(offset) == 'f' ||
			str->at(offset) == 'F' || str->at(offset) == 'l' || str->at(offset) == 'L' || str->at(offset) == 'U' || str->at(offset) == 'E') {
			if ((str->at(offset) == 'e' || str->at(offset) == 'E') && (str->at(offset+1) == '-' || str->at(offset+1) == '+')) {
				offset++;
				tokenLength++;
				if (offset == len) {
					complete = true;
					break;
				}
			}
			offset++;
			tokenLength++;
			if (offset == len) {
				complete = true;
				return;
			}
		}
		if (str->at(offset) == 'x') {//switch to hexadecimal processing
			offset++;
			tokenLength++;
			if(offset == len) {
				complete = true;
				return;
			}
			while(isnumber((char)str->at(offset)) || str->at(offset) == 'A' || str->at(offset) == 'B' || str->at(offset) == 'C' ||
				str->at(offset) == 'D' || str->at(offset) == 'E' || str->at(offset) == 'F' || str->at(offset) == 'L' ||
				str->at(offset) == 'l' || str->at(offset) == 'U') {
				offset++;
				tokenLength++;
				if(offset == len) {
					complete = true;
					break;
				}
			}
		}
		processingFloat = false;
		return;
	}
	else if (processingIncludeStatement) { //this is convoluted, could probably be refactored
		//if include statement, if we don't see a <, ", ', we have to trim whitespace until we see a
		//<,",', or get the entire word "include" or "ifndef" as a token.
		if (str->at(offset) != '<' && str->at(offset) != '"') {
			while(str->at(offset) != ' ' && str->at(offset) != '\t' && str->at(offset) != '<' && str->at(offset) != '"') { 
				//gets the word "include"
				offset++;
				tokenLength++;
				if (offset == len) {
					complete = true;
					return;
				}
			}
		}
		else {
			offset++;
			tokenLength++;
			while(str->at(offset) != '>' && str->at(offset) != '"') {
				offset++; //gets <ctime> or "parserClasses.h" or whatever.
				tokenLength++;
				if (offset == len) {
					complete = true;
					return;
				}
			}
			offset++;
			tokenLength++; //gets the > at the end.
			if (offset == len) {
				complete = true;
			}
		}
		return;
	}
	//begin regular processing
	while(str->at(offset) != ' ' && str->at(offset) != '\t' && offset<len) {
		if (str->at(offset) == '"') { //for string literals 
			tokenLength++;
			offset++;
			while (str->at(offset) != '"' && offset<len) {
				if (str->at(offset) == '\\') { //if it sees one escape sequence, add two to offset and token length.
					tokenLength += 2;
					offset += 2;
					if (offset == len) {
						complete = true; 
						break;
					}
					continue; //check again if it sees a " or ' after the escape sequence.
				}
				tokenLength++;
				offset++; //otherwise just move forward by one.
				if (offset == len) {
					complete = true; 
					break;
				}
			}
			offset++; //includes the last " or the '
			tokenLength++;
			if (offset == len) {
				complete = true; 
				break;
			}
			break;
		}
		else if (str->at(offset) == '\'') { //for string literals 
			tokenLength++;
			offset++;
			while (str->at(offset) != '\'' && offset<len) {
				if (str->at(offset) == '\\') { //if it sees one escape sequence, add two to offset and token length.
					tokenLength += 2;
					offset += 2;
					if (offset == len) {
						complete = true; 
						break;
					}
					continue; //check again if it sees a " or ' after the escape sequence.
				}
				tokenLength++;
				offset++; //otherwise just move forward by one.
				if (offset == len) {
					complete = true; 
					break;
				}
			}
			offset++; //includes the last " or the '
			tokenLength++;
			if (offset == len) {
				complete = true; 
				break;
			}
			break;
		}
		//all operators which can be doubled onto themselves except for --
		//(::, &&, ||, ++, --, <<, >>, and also the <<= and >>= operators
		//and &=, |=, +=, -=, <=, >
		else if (str->at(offset) == '|' || str->at(offset) == '&' || str->at(offset) == '+' || str->at(offset) == '>' ||
			str->at(offset) == '<' || str->at(offset) == ':' || str->at(offset) == '=') {
			offset++;
			tokenLength++;
			if (offset == len) {
				complete = true;
				break;
			}
			if (str->at(offset) == str->at(offset-1) || str->at(offset) == '=') {
				offset++;
				tokenLength++;
				if (offset==len) {
					complete = true;
					break;
				}
				if (str->at(offset) == '=') {
					offset++;
					tokenLength++;
				}
			}
			break;
		}
		else if (str->at(offset) == '*' || str->at(offset) == '%' || str->at(offset) == '!' || str->at(offset) == '~' || 
				str->at(offset) == '.' || str->at(offset) == ',' || str->at(offset) == ';' || str->at(offset) == '^' || 
				str->at(offset) == '?' || str->at(offset) == '#') {
			//list of all operators/specials that come alone/compound.
			if (str->at(offset) == '.') {
				offset++; 
				tokenLength++;
				if (offset == len) {
					complete = true;
					break;
				}
				if (str->at(offset) == '*') {
					offset++;
					tokenLength++;
				}
			}
			else {
				offset++; 
				tokenLength++;
				if (offset == len) { //in case for some reason one of these characters is the end of a line
					complete = true;
					break;
				}
				if (str->at(offset) == '=' || str->at(offset) == ':') {
					//this contains all possible 2nd parts of any compound operators (can't put spaces between)
					//this assumes that syntax of the original file is actually correct (no one wrote something stupid like +&)
					offset++; 
					tokenLength++;
					if (offset == len) { //in case for some reason one of these characters is the end of a line
						complete = true;
						break;
					}
					if (str->at(offset) == '=') {
						//the only 3-
						offset++;
						tokenLength++;
					}
				}
			}
			break;
		}
		else if (str->at(offset) == '-') {
			offset++;
			tokenLength++;
			if (offset == len) {
				complete = true;
				break;
			}
			if (str->at(offset) == '-' || str->at(offset) == '=') {
				offset++;
				tokenLength++;
			}
			else if (str->at(offset) == '>') {
				offset++;
				tokenLength++;
				if (offset == len) {
					complete = true;
					break;
				}
				if (str->at(offset) == '*') {
					offset++;
					tokenLength++;
				}
				break;
			}
			break;
		}
		else if (str->at(offset) == '/') { // slash is special
			offset++;
			tokenLength++;
			if (offset == len) { //in case for some reason one of these characters is the end of a line
				complete = true;
				break;
			}
			if (str->at(offset) == '/' || str->at(offset) == '*' || str->at(offset) == '=') {
				offset++;
				tokenLength++;
				break;
			}
			break;
		}
		else if (str->at(offset) == '(' ||str->at(offset) == ')' || str->at(offset) == '[' || str->at(offset) == ']' ||
				str->at(offset) == '{' || str->at(offset) == '}') {
			offset++;
			tokenLength++;
			if (offset == len) {
				complete = true;
				break;
			}
			break;
		}
		else {
			while (str->at(offset) != '+' && str->at(offset) != '-' && str->at(offset) != '*' && str->at(offset) != '/' &&
				str->at(offset) != '(' && str->at(offset) != '{' && str->at(offset) != '.' && str->at(offset) != '&' && 
				str->at(offset) != ')' && str->at(offset) != '}' && str->at(offset) != ',' && str->at(offset) != '|' && 
				str->at(offset) != '[' && str->at(offset) != '<' && str->at(offset) != ';' && str->at(offset) != '!' && 
				str->at(offset) != ']' && str->at(offset) != '>' && str->at(offset) != ':' && str->at(offset) != '^' && 
				str->at(offset) != '#' && str->at(offset) != '~' && str->at(offset) != '"' && str->at(offset) != '\'' &&
				str->at(offset) != '=' && str->at(offset) != '%' && str->at(offset) != ' ' && str->at(offset) != '\t' &&
				str->at(offset) != '?') {
				//no special characters found, just take more characters.
				offset++;
				tokenLength++;
				if (offset == len) {
					complete = true;
					break;
				}
			}
			break;
		}
		if (offset == len) {
			complete = true;
			break;
		}
	}
	if (offset == len) {
		complete = true;
	}
	return;
}

//Sets the current string to be tokenized
//Resets all Tokenizer state variables
//Calls Tokenizer::prepareNextToken() as the last statement before returning.
//nope it doesn't.
//if we want that to happen, we need getNexttoken to store the current token first before
//calling prepare, and THEN returning.
void Tokenizer::setString(string *str) {
	this->str = str;
	complete = false;
	processingIncludeStatement = false; //include statements end after one line
	processingInlineComment = false; //inline comments end after one line
	processingFloat = false; //float processing cannot be multi-line
	offset = 0;
	tokenLength = 0;
	return;
}

//Returns the next token. Hint: consider the substr function
//Updates the tokenizer state
//Updates offset, resets tokenLength, updates processingABC member variables
//Calls Tokenizer::prepareNextToken() as the last statement before returning.
string Tokenizer::getNextToken() {
	prepareNextToken();
	string d = str->substr(offset-tokenLength,tokenLength);
	if (d == "//") {
		processingInlineComment = true;
	}
	else if (d == "/*") {
		processingBlockComment = true;
	}
	else if (d == "*/") {
		processingBlockComment = false;
	}
	else if (d == "#") {
		processingIncludeStatement = true;
	}
	return d;
}


