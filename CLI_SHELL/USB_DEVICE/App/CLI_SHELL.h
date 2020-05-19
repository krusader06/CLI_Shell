/** @file CLI_SHELL.h
 *
 * @brief Lightweight CLI Shell
 *
 * @author Colton Crandell
 * @revision history:
 * - 1.0: 5-18-2020 (Crandell) Original
 * - 1.1: 5-19-2020 (Crandell)
 * 		Added an extra response code to handle feedback from the function pointer -- "RESPONSE_FNC_ERR"
 * 		Updated Shell Version to 1.1.0
 *
 * COPYRIGHT NOTICE: (c) 2020.  All rights reserved.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef CLI_SHELL_H_
#define CLI_SHELL_H_

/*******************************************************************************
 * INCLUDES
 *******************************************************************************/
#include <stdint.h>
#include <stdbool.h>

#include "usbd_cdc_if.h"

/********************************************************************************
 * DEFINES
 *******************************************************************************/
/**
  * @brief  Shell Version Info
  */

#define SHELL_MAJOR_VER			1
#define SHELL_MINOR_VER			1
#define SHELL_REV				0

/**
  * @brief  Misc Defines
  */

#define MAX_ARGUMENTS			5
#define SHELL_BUFFER_LEN		100					/*!< Alloted Buffer Length				*/

#define SHELL_CMD_LEN			SHELL_BUFFER_LEN	/*!< Alloted Command Length				*/
#define SHELL_ARG_LEN			20					/*!< Alloted Argument Length			*/

/**
  * @brief  When outputStreamChannel() is called within CLI_SHELL.c, it will funnel
  * 		through whatever is defined here
  * @note	This is set up as a USB CDC Interface, so CDC_Transmit_FS will be used.
  */
#define outputStreamChannel(buffer, length)			CDC_Transmit_FS(buffer, length)

/********************************************************************************
 * TYPES
 *******************************************************************************/
// These are only here to accommodate the type references for the below function pointer.
typedef struct shellParserOutputTypeDef	shellParserOutput_t;
typedef enum shellErrorTypeDef shell_error;

// Function Pointer for the function to run for each command.
typedef shell_error(*shellBridge_t)(shellParserOutput_t*);

/**
  * @brief  Argument Tokens. Used to differentiate different arguments within a received command string.
  */
typedef enum {
	argTkn_a = 0,
	argTkn_b,
	argTkn_c,
	argTkn_d,
	argTkn_e,
	argTkn_f,
	argTkn_g,
	argTkn_h,
	argTkn_i,
	argTkn_j,
	argTkn_k,
	argTkn_l,
	argTkn_m,
	argTkn_n,
	argTkn_o,
	argTkn_p,
	argTkn_q,
	argTkn_r,
	argTkn_s,
	argTkn_t,
	argTkn_u,
	argTkn_v,
	argTkn_w,
	argTkn_x,
	argTkn_y,
	argTkn_z,
	argTkn_err
} argToken_t;

/**
  * @brief  Argument Tokens. Used to differentiate different arguments within a received command string.
  */
typedef enum {
	arg_uint8,
	arg_uint16,
	arg_uint32,
	arg_char,
	arg_string,
	arg_float,
	arg_flag,
} argType_t;

/**
  * @brief  Response codes for command/argument error checking.
  */
typedef enum {
	RESPONSE_OK,				/*!< Respond OK			*/
	RESPONSE_FNC_ERR,			/*!< Function Error		*/
	RESPONSE_CMD_ERR,			/*!< Command Error		*/
	RESPONSE_ARG_ERR			/*!< Argument Error		*/
} responseCode_t;

/*------------------------------ PARSER STRUCTURES ------------------------------------*/

/**
  * @brief  Stores one argument from the parser.
  */
typedef struct {
	uint8_t argContents[SHELL_ARG_LEN];		/*!< Pointer to the argument content string	*/
	argToken_t argToken;					/*!< Argument Token							*/
} shellArgument_t;

/**
  * @brief  Stores the output from the parser. Contains the command name and all arguments.
  */
typedef struct shellParserOutputTypeDef {
	uint8_t cmdName[SHELL_CMD_LEN];			/*!< Command Name							*/

	uint8_t numArgs;						/*!< Number of Arguments					*/
	shellArgument_t cmdArgs[MAX_ARGUMENTS];

} shellParserOutput_t;


/*--------------------- COMMAND/ARGUMENT TEMPLATE STRUCTURES --------------------------*/

/**
  * @brief  Template for the Argument Structure. This is used within CLI_SHELL_COMMANDS.h
  * 		and it defines the argument structure.
  */
typedef struct {
	bool mandatory;							/*!< Mandatory Flag							*/
	argType_t type;							/*!< Argument Data Type						*/
	argToken_t token;						/*!< Token to Use							*/
} shellArgTemplate_t;

/**
  * @brief  Template for the Command Structure. This is used within CLI_SHELL_COMMANDS.h
  * 		and it defines the command structure.
  */
typedef struct {
	char* cmdName;							/*!< Pointer to the Command Name			*/
	char* helpDesc;							/*!< Pointer to the Help Description		*/
	shellBridge_t bridge;					/*!< Runner for the associated function		*/
	uint8_t numArgs;						/*!< Number of Arguments					*/
	shellArgTemplate_t cmdArgsTable[MAX_ARGUMENTS];
} shellCmdTemplate_t;

/*------------------------------ GENERAL STRUCTURES ------------------------------------*/
/**
  * @brief  Stores the raw received string, length, and global flags
  */
typedef struct {
	bool rxFlag;

	uint8_t rxBuffer[SHELL_BUFFER_LEN];
	uint32_t rxLen;

} shellBufferHandle_t;

/**
  * @brief  Errors
  */
typedef enum shellErrorTypeDef {
	SHELL_OK,
	SHELL_ERR
} shell_error;

/********************************************************************************
 * PROTOTYPES
 *******************************************************************************/
shell_error shellInit(void);

// Receive a string from the CLI. This is called from the CDC_Receive_FS function.
// It is responsible for loading the structure and
void rxShellInput(uint8_t* Buf, uint32_t *Len);

shell_error checkShellStatus(void);

shell_error HelpBridge(shellParserOutput_t* package);

#endif // CLI_SHELL_H_

/*** end of file ***/






