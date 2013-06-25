/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef __COMMAND_STACK_H__
#define __COMMAND_STACK_H__

#include "Base/BaseTypes.h"
#include "Commands2/Command2.h"
#include "Commands2/CommandBatch.h"

struct CommandStackNotify;

class CommandStack : public CommandNotifyProvider
{
	friend struct CommandStackNotify;

public:
	CommandStack();
	~CommandStack();

	bool CanRedo() const;
	bool CanUndo() const;

	void Clear();
	
	void Undo();
	void Redo();
	void Exec(Command2 *command);

	void BeginBatch(const DAVA::String &text);
	void EndBatch();

	size_t GetUndoLimit() const;
	void SetUndoLimit(size_t limit);

protected:
	std::list<Command2 *> commandList;
	size_t commandListLimit;
	size_t nextCommandIndex;

	CommandBatch* curBatchCommand;
	CommandStackNotify *stackCommandsNotify;

	void ExecInternal(Command2 *command, bool runCommand);

	void ClearRedoCommands();
	void ClearLimitedCommands();

	Command2* GetCommand(size_t index);
	void CommandExecuted(const Command2 *command, bool redo);
};

struct CommandStackNotify : public CommandNotify
{
	CommandStack* stack;

	CommandStackNotify(CommandStack *_stack);
	virtual void Notify(const Command2 *command, bool redo);
};

#endif // __COMMAND_STACK_H__
