/***************************************************************************
	PLaTHEA: People Localization and Tracking for HomE Automation
	Copyright (C) 2014 Francesco Leotta
	
	This file is part of PLaTHEA
	PLaTHEA is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser Public License as published by
	the Free Software Foundation, version 3 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	
	GNU Lesser Public License for more details.

	You should have received a copy of the GNU Lesser Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/

#include "LeoWindowsNetwork.h"
#include <LeoStormSTLExtension.h>

//******** Conversion of sequences of bytes in base64 character string ********//

static const char S_BASE64CHAR[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
			'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
			'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
			'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3',
			'4', '5', '6', '7', '8', '9', '+', '/'};

static const char S_BASE64PAD = '=';

int encodeBase64(char * data, int datalength, char ** out) {
	int dataIndex = 0, outIndex = 0;
	//Every 3 uncoded bytes we produce 4 encoded base64 ASCII character
	//if datalength is not multiple of 3 we have to pad to obtain encoded length multiple of 4
	//the spurious byte is the string end character
	char * output = new char[(datalength/3)*4+4+1];
	while (datalength>=3) {
		int i = (((data[dataIndex]&0xff)<<16) + ((data[dataIndex+1]&0xff)<<8) + (data[dataIndex+2]&0xff));
		output[outIndex++] = S_BASE64CHAR[(i>>18)&0x3f];
		output[outIndex++] = S_BASE64CHAR[(i>>12)&0x3f];
		output[outIndex++] = S_BASE64CHAR[(i>>6)&0x3f];
		output[outIndex++] = S_BASE64CHAR[i&0x3f];
		dataIndex+=3;
		datalength-=3;
	}
	if (datalength==2) {
		int i = (((data[dataIndex]&0xff)<<8) + (data[dataIndex+1]&0xff));
		output[outIndex++] = S_BASE64CHAR[(i>>10)&0x3f];
		output[outIndex++] = S_BASE64CHAR[(i>>4)&0x3f];
		output[outIndex++] = S_BASE64CHAR[(i<<2)&0x3f];
		output[outIndex++] = S_BASE64PAD;
	} else if (datalength==1) {
		int i = data[dataIndex]&0xff;
		output[outIndex++] = S_BASE64CHAR[(i>>2)&0x3f];
		output[outIndex++] = S_BASE64CHAR[(i<<4)&0x3f];
		output[outIndex++] = S_BASE64PAD;
		output[outIndex++] = S_BASE64PAD;
	}
	output[outIndex]='\0';
	*out=output;
	return outIndex;
}

static const signed char S_BASE64Byte[] =   
{
	/* 0 - 31 / 0x00 - 0x1f */
	-1, -1, -1, -1, -1, -1, -1, -1  
	,   -1, -1, -1, -1, -1, -1, -1, -1  
	,   -1, -1, -1, -1, -1, -1, -1, -1  
	,   -1, -1, -1, -1, -1, -1, -1, -1
	/* 32 - 63 / 0x20 - 0x3f */
	,   -1, -1, -1, -1, -1, -1, -1, -1  
	,   -1, -1, -1, 62, -1, -1, -1, 63  /* ... , '+', ... '/' */
	,   52, 53, 54, 55, 56, 57, 58, 59  /* '0' - '7'          */
	,   60, 61, -1, -1, -1, -1, -1, -1  /* '8', '9', ...      */
	/* 64 - 95 / 0x40 - 0x5f */
	,   -1, 0,  1,  2,  3,  4,  5,  6   /* ..., 'A' - 'G'     */
	,   7,  8,  9,  10, 11, 12, 13, 14  /* 'H' - 'O'          */
	,   15, 16, 17, 18, 19, 20, 21, 22  /* 'P' - 'W'          */
	,   23, 24, 25, -1, -1, -1, -1, -1  /* 'X', 'Y', 'Z', ... */
	/* 96 - 127 / 0x60 - 0x7f */
	,   -1, 26, 27, 28, 29, 30, 31, 32  /* ..., 'a' - 'g'     */
	,   33, 34, 35, 36, 37, 38, 39, 40  /* 'h' - 'o'          */
	,   41, 42, 43, 44, 45, 46, 47, 48  /* 'p' - 'w'          */
	,   49, 50, 51, -1, -1, -1, -1, -1  /* 'x', 'y', 'z', ... */

	,   -1, -1, -1, -1, -1, -1, -1, -1  
	,   -1, -1, -1, -1, -1, -1, -1, -1  
	,   -1, -1, -1, -1, -1, -1, -1, -1  
	,   -1, -1, -1, -1, -1, -1, -1, -1  

	,   -1, -1, -1, -1, -1, -1, -1, -1  
	,   -1, -1, -1, -1, -1, -1, -1, -1  
	,   -1, -1, -1, -1, -1, -1, -1, -1  
	,   -1, -1, -1, -1, -1, -1, -1, -1  

	,   -1, -1, -1, -1, -1, -1, -1, -1  
	,   -1, -1, -1, -1, -1, -1, -1, -1  
	,   -1, -1, -1, -1, -1, -1, -1, -1  
	,   -1, -1, -1, -1, -1, -1, -1, -1  

	,   -1, -1, -1, -1, -1, -1, -1, -1  
	,   -1, -1, -1, -1, -1, -1, -1, -1  
	,   -1, -1, -1, -1, -1, -1, -1, -1  
	,   -1, -1, -1, -1, -1, -1, -1, -1  
};

int decodeBase64(char *data, int dataLength, char **out) {
	char *output = new char[(dataLength/4)*3+ 1];
	char *outPtr = output;
	char *dataPtr = data;
	int result = 0;
	while (dataLength > 0) {
		int toAddByte = 3;
		int a = (dataPtr[3] == S_BASE64PAD ? 0 : (S_BASE64Byte[dataPtr[3]] & 0x3f));
		if (dataPtr[3] == S_BASE64PAD)
			toAddByte = 2;
		int b = (dataPtr[2] == S_BASE64PAD ? 0 : ((S_BASE64Byte[dataPtr[2]] & 0x3f) << 6));
		if (dataPtr[2] == S_BASE64PAD)
			toAddByte = 1;
		int c = (dataPtr[1] == S_BASE64PAD ? 0 : ((S_BASE64Byte[dataPtr[1]] & 0x3f) << 12));
		int d = (dataPtr[0] == S_BASE64PAD ? 0 : ((S_BASE64Byte[dataPtr[0]] & 0x3f) << 18));
		int i = a + b + c + d;
		outPtr[0] = (i >> 16) & 0xff;
		outPtr[1] = (i >> 8) & 0xff;
		outPtr[2] = i & 0xff;
		dataLength-=4;
		dataPtr+=4;
		outPtr+=3;
		result+=toAddByte;
	}
	output[result] = '\0';
	*out = output;
	return result;
}

char* createHttpSimpleAuthentication(const char* username, const char*password) {
	char* encodedCredential = NULL;
	char * authString = new char[256];
	sprintf_s(authString, 256, "%s:%s", username, password);
	encodeBase64(authString, (int) strlen(authString), &encodedCredential);
	sprintf_s(authString, 256, "Authorization: Basic %s\r\n", encodedCredential);
	delete[] encodedCredential;
	return authString;
}

//**** Read a single ASCII line from a socket and return it as an STL string ****//
std::string readLine(SOCKET ConnectSocket) {
	std::string res;
	char readed;
	bool moreData = true;
	do {
		int iResult = recv(ConnectSocket, &readed, 1, MSG_WAITALL);
		if (iResult <=0)
			throw 2;
		else if (readed == '\n')
			moreData=false;
		else if (readed != '\r')
			res += readed;
	} while (moreData);
	return res;
}

void TCPRequestAndReply (sockaddr_in* address, char *request, char *reply, int maxcc) {
	SOCKET answerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (answerSocket == INVALID_SOCKET)
		throw LEO_NETWORK_SOCKET_ERROR;
	if (connect(answerSocket, (SOCKADDR *) address, sizeof(sockaddr_in)) == SOCKET_ERROR)
		throw LEO_NETWORK_CONNECT_ERROR;
	send(answerSocket, request, int(strlen(request)), 0);
	int freeSpace = maxcc-1;
	char* freePtr = reply;
	int received;
	while (freeSpace > 0 && ((received = recv(answerSocket, freePtr, freeSpace, 0)) > 0)) {
		freeSpace -= received;
		freePtr += received;
	}
	*freePtr = '\0';
	closesocket(answerSocket);
}

//****** StreamSplitter Implementation (It use STL C++ Library for HashTable implementation *******//

HttpStreamSplitter::HttpStreamSplitter(SOCKET ConnectSocket) {
	this->ConnectSocket = ConnectSocket;
	this->stream_end = false;
}

HttpStreamSplitter::~HttpStreamSplitter() {}

void HttpStreamSplitter::AddPropertyValue(std::string propline, std::unordered_map<std::string, std::string>& ht) {
	std::string::size_type idx = propline.find_first_of(':');
	if (idx == std::string::npos)
		return;
	std::string tag = propline.substr(0, idx);
	std::string val = propline.substr(idx+1);
	trim(val);
	ht[tag] = val;
}

DWORD HttpStreamSplitter::ReadHeaders(std::unordered_map<std::string, std::string> &ht) {
	ht.clear();
	std::string linea;
	bool satisfied = false;
	bool firstLine = true;
	DWORD returnValue = 0;
	do {
		linea = readLine(this->ConnectSocket);
		if (firstLine) {
			returnValue = GetTickCount();
			firstLine = false;
		}
		if (linea.compare(""))
			satisfied = true;
		AddPropertyValue(linea, ht);
	} while (linea.compare("") || !satisfied);
	return returnValue;
}

bool HttpStreamSplitter::IsAtStreamEnd() {
	return stream_end;
}
