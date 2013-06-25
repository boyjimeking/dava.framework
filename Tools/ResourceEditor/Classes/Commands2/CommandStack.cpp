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

#include "Commands2/CommandStack.h"
#include "Commands2/CommandAction.h"

CommandStack::CommandStack()
	: commandListLimit(0)
	, nextCommandIndex(0)
	, curBatchCommand(NULL)
{
	stackCommandsNotify = new CommandStackNotify(this);
}

CommandStack::~CommandStack()
{
	Clear();

	delete stackCommandsNotify;
}

bool CommandStack::CanUndo() const
{
	bool ret = false;

	if(commandList.size() > 0 && nextCommandIndex > 0)
	{
		ret = true;
	}

	return ret;
}

bool CommandStack::CanRedo() const
{
	bool ret = false;

	if(nextCommandIndex < commandList.size())
	{
		ret = true;
	}

	return ret;
}

void CommandStack::Clear()
{
	nextCommandIndex = 0;
	
	std::list<Command2 *>::iterator i = commandList.begin();
	std::list<Command2 *>::iterator end = commandList.end();

	for(; i != end; ++i)
	{
		delete *i;
	}

	commandList.clear();
}

void CommandStack::Undo()
{
	if(CanUndo())
	{
		nextCommandIndex--;

		Command2* commandToUndo = GetCommand(nextCommandIndex);
		if(NULL != commandToUndo)
		{
			commandToUndo->Undo();
			EmitNotify(commandToUndo, false);
		}
	}
}

void CommandStack::Redo()
{
	if(CanRedo())
	{
		Command2* commandToRedo = GetCommand(nextCommandIndex);
		if(NULL != commandToRedo)
		{
			commandToRedo->Redo();
			EmitNotify(commandToRedo, true);
		}

		nextCommandIndex++;
	}
}

void CommandStack::Exec(Command2 *command)
{
	if(NULL != command)
	{
		CommandAction* action = dynamic_cast<CommandAction*>(command);
		if (!action)
		{
			if(NULL != curBatchCommand)
			{
				curBatchCommand->Add(command);
			}
			else
			{
				ExecInternal(command);
			}
		}
		else
		{
			action->Redo();
			delete action;
		}
	}
}

void CommandStack::BeginBatch(const DAVA::String &text)
{
	if(NULL == curBatchCommand)
	{
		curBatchCommand = new CommandBatch();
		curBatchCommand->SetText(text);
	}
}

void CommandStack::EndBatch()
{
	if(NULL != curBatchCommand)
	{
		if(curBatchCommand->Size() > 0)
		{
			ExecInternal(curBatchCommand);
		}
		else
		{
			delete curBatchCommand;
		}

		curBatchCommand = NULL;
	}
}

size_t CommandStack::GetUndoLimit() const
{
	return commandListLimit;
}

void CommandStack::SetUndoLimit(size_t limit)
{
	commandListLimit = limit;
}

void CommandStack::ExecInternal(Command2 *command)
{
	ClearRedoCommands();

	commandList.push_back(command);
	nextCommandIndex++;

	command->SetNotify(stackCommandsNotify, false);
	command->Redo();
	EmitNotify(command, true);

	ClearLimitedCommands();
}

void CommandStack::ClearRedoCommands()
{
	if(CanRedo())
	{
		std::list<Command2 *>::iterator i = commandList.begin();
		std::list<Command2 *>::iterator next;

		std::advance(i, nextCommandIndex);
		while(i != commandList.end())
		{
			next = i;
			next++;

			delete *i;
			commandList.erase(i);

			i = next;
		}
	}
}

void CommandStack::ClearLimitedCommands()
{
	while(commandListLimit > 0 && commandList.size() > commandListLimit)
	{
		Command2* command = GetCommand(0);

		// this is single command
		if(NULL != command)
		{
			delete command;
		}

		commandList.pop_front();
		nextCommandIndex--;
	}
}

Command2* CommandStack::GetCommand(size_t index)
{
	Command2* command = NULL;

	if(index < commandList.size())
	{
		std::list<Command2 *>::iterator i = commandList.begin();
		std::advance(i, index);

		if(i != commandList.end())
		{
			command = *i;
		}
	}

	return command;
}

void CommandStack::CommandExecuted(const Command2 *command, bool redo)
{
	EmitNotify(command, redo);
}


CommandStackNotify::CommandStackNotify(CommandStack *_stack)
	: stack(_stack)
{ }

void CommandStackNotify::Notify(const Command2 *command, bool redo)
{
	if(NULL != stack)
	{
		stack->CommandExecuted(command, redo);
	}
}
