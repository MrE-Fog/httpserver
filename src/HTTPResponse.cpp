/**
 ByteBuffer
 HTTPResponse.cpp
 Copyright 2011-2021 Ramsey Kant

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#include "HTTPMessage.h"
#include "HTTPResponse.h"

HTTPResponse::HTTPResponse() : HTTPMessage() {
    this->init();
}

HTTPResponse::HTTPResponse(std::string sData) : HTTPMessage(sData) {
    this->init();
}

HTTPResponse::HTTPResponse(byte* pData, unsigned int len) : HTTPMessage(pData, len) {
    this->init();
}

HTTPResponse::~HTTPResponse() {
}

void HTTPResponse::init() {
    status = 0;
    reason = "";
}

/**
 * Determine the status code based on the parsed Responses reason string
 * The reason string is non standard so this method needs to change in order to handle
 * responses with different kinds of strings
*/
void HTTPResponse::determineStatusCode() {
    if (reason.find("Continue") != std::string::npos) {
        status = Status(CONTINUE);
    } else if (reason.find("OK") != std::string::npos) {
        status = Status(OK);
    } else if (reason.find("Bad Request") != std::string::npos) {
        status = Status(BAD_REQUEST);
    } else if (reason.find("Not Found") != std::string::npos) {
        status = Status(NOT_FOUND);
    } else if (reason.find("Server Error") != std::string::npos) {
        status = Status(SERVER_ERROR);
    } else if (reason.find("Not Implemented") != std::string::npos) {
        status = Status(NOT_IMPLEMENTED);
    } else {
    }
}

/**
 * Determine the reason string based on the Response's status code
*/
void HTTPResponse::determineReasonStr() {
    switch (status) {
    case Status(CONTINUE):
        reason = "Continue";
        break;
    case Status(OK):
        reason = "OK";
        break;
    case Status(BAD_REQUEST):
        reason = "Bad Request";
        break;
    case Status(NOT_FOUND):
        reason = "Not Found";
        break;
    case Status(SERVER_ERROR):
        reason = "Internal Server Error";
        break;
    case Status(NOT_IMPLEMENTED):
        reason = "Not Implemented";
        break;
    default:
        break;
    }
}

/**
 * Create
 * Create and return a byte array of an HTTP response, built from the variables of this HTTPResponse
 * Caller will be responsible for cleaning up returned byte array
 *
 * @return Byte array of this HTTPResponse to be sent over the wire
 */
byte* HTTPResponse::create() {
    // Clear the bytebuffer in the event this isn't the first call of create()
    clear();

    // Insert the status line: <version> <status code> <reason>\r\n
    std::stringstream sline;
    sline << version << " " << status << " " << reason;
    putLine(sline.str());

    // Put all headers
    putHeaders();

    // If theres body data, add it now
    if ((data != NULL) && dataLen > 0) {
        putBytes(data, dataLen);
    }

    // Allocate space for the returned byte array and return it
    byte* createRetData = new byte[size()];
    setReadPos(0);
    getBytes(createRetData, size());

    return createRetData;
}

/**
 * Parse
 * Populate internal HTTPResponse variables by parsing the HTTP data
 *
 * @param True if successful. If false, sets parseErrorStr for reason of failure
 */
bool HTTPResponse::parse() {
    std::string statusstr;

    // Get elements from the status line: <version> <status code> <reason>\r\n
    version = getStrElement();
    statusstr = getStrElement();
    determineStatusCode();
    reason = getLine(); // Pull till \r\n termination

    // Parse and populate the headers map using the parseHeaders helper
    parseHeaders();

    // If the body of the message
    if (!parseBody())
        return false;

    return true;
}


