/*
 * Copyright (c) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2012 Research In Motion Limited. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "ScriptCallStackFactory.h"

#include "InspectorInstrumentation.h"
#include "JSDOMBinding.h"
#include "JSMainThreadExecState.h"
#include "ScriptArguments.h"
#include "ScriptCallFrame.h"
#include "ScriptCallStack.h"
#include "ScriptValue.h"
#include <interpreter/CallFrame.h>
#include <interpreter/Interpreter.h>
#include <runtime/ArgList.h>
#include <runtime/JSCJSValue.h>
#include <runtime/JSFunction.h>
#include <runtime/JSGlobalData.h>
#include <wtf/text/WTFString.h>

using namespace JSC;

namespace WebCore {

class ScriptExecutionContext;

PassRefPtr<ScriptCallStack> createScriptCallStack(size_t maxStackSize, bool emptyIsAllowed)
{
    Vector<ScriptCallFrame> frames;
    if (JSC::ExecState* exec = JSMainThreadExecState::currentState()) {
        Vector<StackFrame> stackTrace;
        Interpreter::getStackTrace(&exec->globalData(), stackTrace, maxStackSize);
        for (size_t i = 0; i < stackTrace.size(); i++)
            frames.append(ScriptCallFrame(stackTrace[i].friendlyFunctionName(exec), stackTrace[i].friendlySourceURL(), stackTrace[i].line(), stackTrace[i].column()));
    }
    if (frames.isEmpty() && !emptyIsAllowed) {
        // No frames found. It may happen in the case where
        // a bound function is called from native code for example.
        // Fallback to setting lineNumber to 0, and source and function name to "undefined".
        frames.append(ScriptCallFrame("undefined", "undefined", 0, 0));
    }
    return ScriptCallStack::create(frames);
}

PassRefPtr<ScriptCallStack> createScriptCallStack(JSC::ExecState* exec, size_t maxStackSize)
{
    Vector<ScriptCallFrame> frames;
    Vector<StackFrame> stackTrace;
    Interpreter::getStackTrace(&exec->globalData(), stackTrace, maxStackSize + 1);
    for (size_t i = stackTrace.size() == 1 ? 0 : 1; i < stackTrace.size(); i++) {
        // This early exit is necessary to maintain our old behaviour
        // but the stack trace we produce now is complete and handles all
        // ways in which code may be running
        if (!stackTrace[i].callee && frames.size())
            break;

        String functionName = stackTrace[i].friendlyFunctionName(exec);
        frames.append(ScriptCallFrame(functionName, stackTrace[i].sourceURL, stackTrace[i].line(), stackTrace[i].column()));
    }

    return ScriptCallStack::create(frames);
}

PassRefPtr<ScriptCallStack> createScriptCallStackForConsole(JSC::ExecState* exec)
{
    size_t maxStackSize = 1;
    if (InspectorInstrumentation::hasFrontends()) {
        ScriptExecutionContext* scriptExecutionContext = jsCast<JSDOMGlobalObject*>(exec->lexicalGlobalObject())->scriptExecutionContext();
        if (InspectorInstrumentation::consoleAgentEnabled(scriptExecutionContext))
            maxStackSize = ScriptCallStack::maxCallStackSizeToCapture;
    }
    return createScriptCallStack(exec, maxStackSize);
}

PassRefPtr<ScriptArguments> createScriptArguments(JSC::ExecState* exec, unsigned skipArgumentCount)
{
    Vector<ScriptValue> arguments;
    size_t argumentCount = exec->argumentCount();
    for (size_t i = skipArgumentCount; i < argumentCount; ++i)
        arguments.append(ScriptValue(exec->globalData(), exec->argument(i)));
    return ScriptArguments::create(exec, arguments);
}

} // namespace WebCore
