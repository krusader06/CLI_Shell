/** @file CLI_SHELL_COMMANDS.h
 *
 * @brief Command List for the Lightweight CLI Shell
 *
 * @author Colton Crandell
 * @revision history:
 * - 1.0: 5-18-2020 (Crandell) Original
 *
 * To Add Commands:
 *  1. Copy the command template below and paste a copy where desired (Must be within the table)
 *  2. Type the desired command
 *  3. Type the Help Description. This description will be listed whenever the help command is received.
 *  4. Input the runner. This is the function that will be called when the command is received.
 *  5. Input any arguments if necessary and update the number of arguments.
 *  6. Update the global define "NUM_OF_COMMANDS" to the total number of commands within the table.
 *
 * COPYRIGHT NOTICE: (c) 2020.  All rights reserved.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef CLI_SHELL_COMMANDS_H_
#define CLI_SHELL_COMMANDS_H_

/*******************************************************************************
 * INCLUDES
 *******************************************************************************/
#include "CLI_SHELL.h"

/********************************************************************************
 * DEFINES
 *******************************************************************************/
#define NUM_OF_COMMANDS			3

/********************************************************************************
 * COMMAND LIST
 *******************************************************************************/

shellCmdTemplate_t shellCmdTemplateTable[NUM_OF_COMMANDS] = {

		/*-------------------------------------------------*/
		/*-----------------Help Commands-------------------*/
		/*-------------------------------------------------*/
		{
				.cmdName = "help",
				.helpDesc = "help\t| Display the Help Menu\t| No Arguments\r\n",
				.runner = HelpRunner,
				.numArgs = 0,
				.cmdArgsTable = {},
		},
		{
				.cmdName = "?",
				.helpDesc = "?\t| Display the Help Menu\t| No Arguments\r\n",
				.runner = HelpRunner,
				.numArgs = 0,
				.cmdArgsTable = {},
		},

		/*-------------------------------------------------*/
		/*-----------------Ping Command--------------------*/
		/*-------------------------------------------------*/
		{
				.cmdName = "ping",
				.helpDesc = "ping\t| Responds \"Pong!\"\t| No Arguments\r\n",
				.runner = PingRunner,
				.numArgs = 0,
				.cmdArgsTable = {},
		},


//		/*-------------------------------------------------*/
//		/*---------------Command Template------------------*/
//		/*-------------------------------------------------*/
//		{
//				.cmdName = "commandName",
//				.helpDesc = "commandName\t| Input Description\t| List Arguments\r\n",
//				.runner = <Function to Run>,
//				.numArgs = 2,
//				.cmdArgsTable = {
//						{
//								.mandatory = true,
//								.type = arg_uint8,
//								.token = argTkn_a,
//						},
//						{
//								.mandatory = true,
//								.type = arg_uint8,
//								.token = argTkn_b,
//						},
//				},
//		},





};



#endif // CLI_SHELL_COMMANDS_H_

/*** end of file ***/
