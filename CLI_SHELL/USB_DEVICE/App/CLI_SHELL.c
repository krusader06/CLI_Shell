/** @file CLI_SHELL.c
 *
 * @brief Lightweight CLI Shell
 *
 * @author Colton Crandell
 * @revision history:
 * - 1.0: 5-18-2020 (Crandell) Original
 *
 * Usage and Installation Notes:
 *  - "rxShellInput()" should be called when a command has been issued. In the case of USB CLI,
 * 		this function should be called within CDC_Receive_FS() function within usbd_cdc_if.c
 *  - The main loop should call "checkShellStatus()" periodically. If a command has been sent,
 * 		this function will service the command.
 *  - This module is designed to be light weight and will run within a non-OS environment - RTOS is not supported.
 *  - To add commands, see CLI_SHELL_COMMANDS.h
 *
 * COPYRIGHT NOTICE: (c) 2020.  All rights reserved.
 */

/*******************************************************************************
 * INCLUDES
 *******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "CLI_SHELL_COMMANDS.h"
#include "CLI_SHELL.h"

/********************************************************************************
 * DEFINES
 *******************************************************************************/


/********************************************************************************
 * MODULAR VARIABLES
 *******************************************************************************/
bool cliShellInitialized = false;

shellBufferHandle_t shellBuffer;		/*!< Global Command Buffer Storage		*/

/********************************************************************************
 * PRIVATE PROTOTYPES
 *******************************************************************************/
shell_error scrubWhiteSpace(void);
argToken_t getTokenFromChar(char chrToken);
shell_error exctractCommand(shellParserOutput_t* cmdParseOut);
shell_error extractArguments(shellParserOutput_t* cmdParseOut);

/*------------------------------------------------------------------------------*/
bool validateArgType(argType_t argDataType, uint8_t* dataString);
bool validateArgs(shellParserOutput_t* cmdParserOutput, uint8_t commandIndex);
shell_error matchCommand(shellParserOutput_t* cmdParserOutput, int8_t* commandIndex);
shell_error getCommand(shellParserOutput_t* cmdParserOutput, uint8_t* commandTableIndex);

/*------------------------------------------------------------------------------*/
shell_error shellProcessCommand(void);
shell_error shellParseCommand(shellParserOutput_t* cmdParseOut);
shell_error shellSendResponse(responseCode_t code);


/********************************************************************************
 * PRIVATE FUNCTIONS
 *******************************************************************************/
/**
  * @brief  Scrubs all whitespace from the received command
  * @param  NONE
  * @retval shell_error Error Return Value
  */
shell_error scrubWhiteSpace(void) {
	shell_error status = SHELL_OK;

	// Get the current Length of the Buffer
	uint8_t currentLength = shellBuffer.rxLen;

	// Scrub Leading Whitespace
	while (shellBuffer.rxBuffer[0] == ' ' && currentLength > 0) {
		/* Shift Array to the Left */
		for (uint8_t i = 0; i < currentLength - 1; i++) {
			shellBuffer.rxBuffer[i] = shellBuffer.rxBuffer[i + 1];
		}
		currentLength--;
	}

	// Scrub Trailing Whitespace
	uint8_t lastIndex = currentLength - 1;
	while (shellBuffer.rxBuffer[lastIndex] == ' ' && currentLength > 0) {
		currentLength--;
		lastIndex = currentLength - 1;
	}

	// Scrub Duplicate Whitespace
	for (uint8_t i = 0; i < currentLength; i++) {
		if (shellBuffer.rxBuffer[i] == ' ' && shellBuffer.rxBuffer[i + 1] == ' ') {
			for (uint8_t j = (i + 1); j < currentLength -2; j++) {
				shellBuffer.rxBuffer[j] = shellBuffer.rxBuffer[j + 1];
			}
			currentLength--;
		}
	}

	// Update the received Length
	shellBuffer.rxLen = currentLength;
	return status;
}

/**
  * @brief  Converts a single character into a token assignment
  * @param[IN] chrToken single lowercase character (a to z)
  * @retval argToken_t Returns an Argument Token
  */
argToken_t getTokenFromChar(char chrToken) {
	uint8_t asciiVal = chrToken;

	if (asciiVal >= 97 && asciiVal <= 122) {
		// Valid Token
		return (argToken_t)(asciiVal - 97);
	} else {
		return argTkn_err;
	}
}


/**
  * @brief  Extracts the command from the buffer and loads it into the parser output structure
  * @param [IN] cmdParseOut	Pointer to the parser output structure
  * @retval shell_error Error Return Value
  */
shell_error exctractCommand(shellParserOutput_t* cmdParseOut) {
	shell_error status = SHELL_OK;
	uint8_t bufferCopy[SHELL_CMD_LEN] = {0};

	// Delimeter token for strtok
	const char delim[2] = " ";

	// Make a copy of the buffer to preserve.
	memcpy(bufferCopy, shellBuffer.rxBuffer, shellBuffer.rxLen);

	// The command still has the arguments attached.
	// Split the string using ' ' as a delimiter
	char* extractCmd = strtok((char*)bufferCopy, delim);

	// Copy the command name to the parser output
	sprintf((char*)cmdParseOut->cmdName, "%s", extractCmd);

	return status;
}

/**
  * @brief  Extracts the arguments from the buffer and loads them into the parser output structure
  * @param [IN] cmdParseOut	Pointer to the parser output structure
  * @retval shell_error Error Return Value
  */
shell_error extractArguments(shellParserOutput_t* cmdParseOut) {
	shell_error status = SHELL_OK;
	uint8_t bufferCopy[SHELL_CMD_LEN] = {0};

	// Delimiter token for strtok
	const char delim[2] = " ";

	// Make a copy of the buffer to preserve
	memcpy(bufferCopy, shellBuffer.rxBuffer, shellBuffer.rxLen);

	// This first one will extract the command. We don't care about this - the next use of strtok will overwrite this.
	char* extractArg = strtok((char*)bufferCopy, delim);
	extractArg = strtok(NULL, delim);

	uint8_t i = 0;
	while (i < MAX_ARGUMENTS && extractArg != NULL){
		// Copy The Argument
		memset(cmdParseOut->cmdArgs[i].argContents, 0, strlen(extractArg) + 1);
		memcpy(cmdParseOut->cmdArgs[i].argContents, extractArg, strlen(extractArg));

		// Copy the Length. We don't care about the token
		cmdParseOut->cmdArgs[i].argContLen = strlen(extractArg) - 1;

		// Find and assign the argument token
		cmdParseOut->cmdArgs[i].argToken = getTokenFromChar(extractArg[0]);

		// Extract each argument individually
		extractArg = strtok(NULL, delim);

		// Iterate through all possible arguments
		i++;
	}

	// Update the total number of arguments found
	cmdParseOut->numArgs = i;

	return status;
}

/*------------------------------------------------------------------------------*/

/**
  * @brief  Validates the data type using the argument content string.
  * @param[IN]  argDataType Valid Data Type
  * @param[IN]	dataString Input argument content string
  * @retval bool Returns true if the argument content string matches the data type
  */
bool validateArgType(argType_t argDataType, uint8_t* dataString) {
	long ret;

	switch (argDataType) {
		case arg_uint8:
			// Valid = 0 to 0xFF
			// Convert the string to an unsigned integer
			ret = strtol((const char*)dataString, NULL, 10);
			if (ret < 0 && ret > 0xFF) {
				return false;
			}
			break;

		case arg_uint16:
			// Valid = 0 to 0xFFFF
			// Convert the string to an unsigned integer
			ret = strtol((const char*)dataString, NULL, 10);
			if (ret < 0 && ret > 0xFFFF) {
				return false;
			}
			break;

		case arg_uint32:
			// Valid = 0 to 0XFFFFFFFF
			// Convert the string to an unsigned integer
			ret = strtol((const char*)dataString, NULL, 10);
			if (ret < 0 && ret > 0xFFFFFFFF) {
				return false;
			}
			break;

		case arg_char:
			// Valid = 32 to 127 Ascii Character (space until DEL)
			ret = (char)*dataString;
			if (ret < 32 && ret > 127) {
				return false;
			}
			break;

		case arg_string:

			break;

		case arg_float:

			break;

		case arg_flag:

			break;
	}

	return true;
}

/**
  * @brief  Validates the arguments.
  * @note 	There are a couple of checks that happen during this function:
  * 		1. Steps through all mandatory arguments and locates within the input arguments based on the token
  * 		It then validates each mandatory argument based on the required input type
  * 		2. Steps through all input arguments and tries to find any un-needed or mis-typed arguments based on token
  * @param[IN]  cmdParserOutput Parser Output Structure that holds all command/argument info
  * @param[IN]	commandIndex Index of the command within the Command Table.
  * @retval bool Returns true if all arguments are valid.
  */
bool validateArgs(shellParserOutput_t* cmdParserOutput, uint8_t commandIndex) {
	bool tokenFound;

	// Loop through and find all mandatory arguments
	for (uint8_t i = 0; i < shellCmdTemplateTable[commandIndex].numArgs; i++) {
		if (shellCmdTemplateTable[commandIndex].cmdArgsTable[i].mandatory == true) {
			// The argument is mandatory. Try to find the token in the parser output
			tokenFound = false;
			for (uint8_t j = 0; j < cmdParserOutput->numArgs; j++) {
				if (cmdParserOutput->cmdArgs[j].argToken == shellCmdTemplateTable[commandIndex].cmdArgsTable[i].token) {
					tokenFound = true;

					// Found the token. Now we just need to confirm that the data type works.
					if (!validateArgType(shellCmdTemplateTable[commandIndex].cmdArgsTable[i].type, cmdParserOutput->cmdArgs[j].argContents)) {
						// The data type conflicts
						return false;
					}
				}
			}

			if (!tokenFound) {
				// A required token is not present
				return false;
			}
		}
	}
	return true;
}

/**
  * @brief  Tries to locate and match the command within the Command Table
  * @param[IN]  cmdParserOutput Parser Output Structure that holds all command/argument info
  * @param[OUT]	commandIndex Index of the command within the Command Table. If it can't find
  * 			a match, this returns a -1.
  * @retval shell_error Error Return Value
  */
shell_error matchCommand(shellParserOutput_t* cmdParserOutput, int8_t* commandIndex) {
	shell_error status = SHELL_OK;
	int ret;
	bool matchFlag = false;

	// Check each command name
	for (uint8_t i = 0; i < NUM_OF_COMMANDS; i++) {
		ret = strcmp((const char*)cmdParserOutput->cmdName, shellCmdTemplateTable[i].cmdName);
		if (ret == 0) {
			// Matched. Set commandIndex to the current command.
			matchFlag = true;
			*commandIndex = i;
		}
	}

	if (matchFlag == false) {
		// We did not find a match
		*commandIndex = -1;
	}

	return status;
}

/**
  * @brief  Matches the command then validates the arguments.
  * @param[IN]  cmdParserOutput Parser Output Structure that holds all command/argument info
  * @param[OUT]	commandTableIndex Index of the command within the Command Table.
  * @retval shell_error Error Return Value
  */
shell_error getCommand(shellParserOutput_t* cmdParserOutput, uint8_t* commandTableIndex) {
	shell_error status = SHELL_OK;

	// Find the command within the Command Table
	int8_t commandIndex;
	matchCommand(cmdParserOutput, &commandIndex);

	if (commandIndex < 0) {
		// Couldn't find the command
		shellSendResponse(RESPONSE_CMD_ERR);
		return SHELL_ERR;
	}

	// A command has been found. Validate the arguments.
	if (!validateArgs(cmdParserOutput, commandIndex)) {
		// Could not validate arguments
		shellSendResponse(RESPONSE_ARG_ERR);
		return SHELL_ERR;
	}

	// Associate the correct command
	*commandTableIndex = commandIndex;
	return status;
}

/**
  * @brief  Frees the memory
  * @param[IN]  cmdParserOutput Parser Output Structure that holds all command/argument info
  * @param[OUT]	commandTableIndex Index of the command within the Command Table.
  * @retval shell_error Error Return Value
  */
shell_error destroyParserOutput(shellParserOutput_t* cmdParserOutput) {
	shell_error status = SHELL_OK;

	// The only memory that hasn't been freed is the argument contents
	for (uint8_t i = 0; i < MAX_ARGUMENTS; i++) {
		if (cmdParserOutput->cmdArgs[i].argContents != NULL) {
			free(cmdParserOutput->cmdArgs[i].argContents);
		}
	}

	return status;
}

/*------------------------------------------------------------------------------*/

/**
  * @brief  Handles the command when received.
  * @note	Performs all tasks from parsing to command function execution
  * @param  NONE
  * @retval shell_error Error Return Value
  */
shell_error shellProcessCommand(void) {
	shell_error status;

	shellParserOutput_t parserOutput;

	// Step 1. Parse the Command to separate the command from the arguments
	status = shellParseCommand(&parserOutput);
	if (status != SHELL_OK){
		return status;
	}

	// Step 2. Find the correct command and verify the arguments
	uint8_t commandTableIndex;
	status = getCommand(&parserOutput, &commandTableIndex);
	if (status != SHELL_OK){
		return status;
	}

	// Step 3. Fetch and run the associated function
	shellCmdTemplateTable[commandTableIndex].runner(&parserOutput);

	return status;
}

/**
  * @brief  Parses a received command line and outputs to a shellParserOutput structure.
  * @param[Out]  cmdParseOut Pointer to the parser output structure.
  * @retval shell_error Error Return Value
  */
shell_error shellParseCommand(shellParserOutput_t* cmdParseOut) {
	shell_error status;
	// Take care of any extra whitespace
	scrubWhiteSpace();

	// Extract the command name and length and store into the parser output
	status = exctractCommand(cmdParseOut);
	if (status != SHELL_OK) {
		return status;
	}

	// For each argument, get the token and store the argument content/length
	status = extractArguments(cmdParseOut);
	if (status != SHELL_OK) {
		return status;
	}

	return status;
}

/**
  * @brief  Sends an error response based on the command result
  * @param[In]  code Result Code to send
  * @retval shell_error Error Return Value
  */
shell_error shellSendResponse(responseCode_t code) {
	shell_error status = SHELL_OK;
	char tmpBuffer[30] = {0};

	switch (code) {
	case RESPONSE_OK:
		strcpy(tmpBuffer, "-->OK!\r\n");
		outputStreamChannel((uint8_t*)tmpBuffer, strlen(tmpBuffer));
		break;

	case RESPONSE_CMD_ERR:
		strcpy(tmpBuffer, "Command Error!\r\n");
		outputStreamChannel((uint8_t*)tmpBuffer, strlen(tmpBuffer));
		break;

	case RESPONSE_ARG_ERR:
		strcpy(tmpBuffer, "Argument Error!\r\n");
		outputStreamChannel((uint8_t*)tmpBuffer, strlen(tmpBuffer));
		break;

	}

	return status;
}


/********************************************************************************
 * PUBLIC FUNCTIONS
 *******************************************************************************/
shell_error shellInit(void) {
	cliShellInitialized = true;
	return SHELL_OK;
}

/**
  * @brief  Receive and prepares a CLI string
  * @note	This is called from the CDC_Receive_FS function. It quickly stores the string into the cliHandler structure.
  * 		The flag is set so that the main program can address the command when ready.
  * @param  Buf Pointer to the received CLI string
  * @param  Len Pointer to the length of the received string
  * @retval NONE
  */
void rxShellInput(uint8_t* Buf, uint32_t *Len) {
	if (Buf[0] != 13){					// Ignore Return Character
		shellBuffer.rxFlag = true;		/* Set the flag for the parser					*/
		shellBuffer.rxLen = Len[0];		/* Store the command length for the parser		*/

		// Copy the Received String to the Buffer in the structure
		memcpy(shellBuffer.rxBuffer, Buf, shellBuffer.rxLen);
	}
}

/**
  * @brief  Checks the receive status, parses, and executes any received command.
  * @note	This should be called periodically from the main loop.
  * @param  NONE
  * @retval shell_error Error Return Value
  */
shell_error checkShellStatus(void) {
	shell_error status;

	if (shellBuffer.rxFlag) {
		// We received Something - Process it.
		status = shellProcessCommand();

		// Reset the flag after parsing
		shellBuffer.rxFlag = false;
	}
	return status;
}

/**
  * @brief  Iterates through each command and prints the help descriptions
  * @note	Commands located within CLI_SHELL_COMMANDS.h
  * @note   The outputStreamChannel is defined in CLI_SHELL.h
  * @param[IN]  parserInput	snapshot input from the command line parser
  * @retval shell_error Error Return Value
  */
shell_error HelpRunner(shellParserOutput_t* parserInput) {
	shell_error status = SHELL_OK;
	char tmpBuffer[500] = {0};

	// Print the Header
	sprintf(tmpBuffer, "<-- Shell Debug Kernel -->\r\n<-- Rev: %02d.%02d.%02d      -->\r\nCommand\t| Description\t\t| Arguments\r\n\r\n", SHELL_MAJOR_VER, SHELL_MINOR_VER, SHELL_REV);

	// Step through each command, adding help text to the buffer
	for (uint8_t i = 0; i < NUM_OF_COMMANDS; i++) {
		strcat((char*)tmpBuffer, shellCmdTemplateTable[i].helpDesc);
	}

	// Find the length of the string and send it.
	uint16_t bufLen = strlen((const char*)tmpBuffer);
	outputStreamChannel((uint8_t*)tmpBuffer, bufLen);

	return status;
}

shell_error PingRunner(shellParserOutput_t* package) {
	shell_error status = SHELL_OK;
	outputStreamChannel((uint8_t*)"Pong!\r\n", 7);
	return status;
}

/*** end of file ***/
