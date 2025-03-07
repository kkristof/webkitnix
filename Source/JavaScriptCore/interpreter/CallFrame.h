/*
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003, 2007, 2008, 2011 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef CallFrame_h
#define CallFrame_h

#include "AbstractPC.h"
#include "JSGlobalData.h"
#include "JSStack.h"
#include "MacroAssemblerCodeRef.h"
#include "Register.h"

namespace JSC  {

    class Arguments;
    class JSActivation;
    class Interpreter;
    class JSScope;

    // Represents the current state of script execution.
    // Passed as the first argument to most functions.
    class ExecState : private Register {
    public:
        JSValue calleeAsValue() const { return this[JSStack::Callee].jsValue(); }
        JSObject* callee() const { return this[JSStack::Callee].function(); }
        CodeBlock* codeBlock() const { return this[JSStack::CodeBlock].Register::codeBlock(); }
        JSScope* scope() const
        {
            ASSERT(this[JSStack::ScopeChain].Register::scope());
            return this[JSStack::ScopeChain].Register::scope();
        }

        // Global object in which execution began.
        JSGlobalObject* dynamicGlobalObject();

        // Global object in which the currently executing code was defined.
        // Differs from dynamicGlobalObject() during function calls across web browser frames.
        JSGlobalObject* lexicalGlobalObject() const;

        // Differs from lexicalGlobalObject because this will have DOM window shell rather than
        // the actual DOM window, which can't be "this" for security reasons.
        JSObject* globalThisValue() const;

        JSGlobalData& globalData() const;

        // Convenience functions for access to global data.
        // It takes a few memory references to get from a call frame to the global data
        // pointer, so these are inefficient, and should be used sparingly in new code.
        // But they're used in many places in legacy code, so they're not going away any time soon.

        void clearException() { globalData().exception = JSValue(); }
        void clearSupplementaryExceptionInfo()
        {
            globalData().exceptionStack = RefCountedArray<StackFrame>();
        }

        JSValue exception() const { return globalData().exception; }
        bool hadException() const { return globalData().exception; }

        const CommonIdentifiers& propertyNames() const { return *globalData().propertyNames; }
        const MarkedArgumentBuffer& emptyList() const { return *globalData().emptyList; }
        Interpreter* interpreter() { return globalData().interpreter; }
        Heap* heap() { return &globalData().heap; }
#ifndef NDEBUG
        void dumpCaller();
#endif
        static const HashTable* arrayConstructorTable(CallFrame* callFrame) { return callFrame->globalData().arrayConstructorTable; }
        static const HashTable* arrayPrototypeTable(CallFrame* callFrame) { return callFrame->globalData().arrayPrototypeTable; }
        static const HashTable* booleanPrototypeTable(CallFrame* callFrame) { return callFrame->globalData().booleanPrototypeTable; }
        static const HashTable* dateTable(CallFrame* callFrame) { return callFrame->globalData().dateTable; }
        static const HashTable* dateConstructorTable(CallFrame* callFrame) { return callFrame->globalData().dateConstructorTable; }
        static const HashTable* errorPrototypeTable(CallFrame* callFrame) { return callFrame->globalData().errorPrototypeTable; }
        static const HashTable* globalObjectTable(CallFrame* callFrame) { return callFrame->globalData().globalObjectTable; }
        static const HashTable* jsonTable(CallFrame* callFrame) { return callFrame->globalData().jsonTable; }
        static const HashTable* mathTable(CallFrame* callFrame) { return callFrame->globalData().mathTable; }
        static const HashTable* numberConstructorTable(CallFrame* callFrame) { return callFrame->globalData().numberConstructorTable; }
        static const HashTable* numberPrototypeTable(CallFrame* callFrame) { return callFrame->globalData().numberPrototypeTable; }
        static const HashTable* objectConstructorTable(CallFrame* callFrame) { return callFrame->globalData().objectConstructorTable; }
        static const HashTable* privateNamePrototypeTable(CallFrame* callFrame) { return callFrame->globalData().privateNamePrototypeTable; }
        static const HashTable* regExpTable(CallFrame* callFrame) { return callFrame->globalData().regExpTable; }
        static const HashTable* regExpConstructorTable(CallFrame* callFrame) { return callFrame->globalData().regExpConstructorTable; }
        static const HashTable* regExpPrototypeTable(CallFrame* callFrame) { return callFrame->globalData().regExpPrototypeTable; }
        static const HashTable* stringConstructorTable(CallFrame* callFrame) { return callFrame->globalData().stringConstructorTable; }

        static CallFrame* create(Register* callFrameBase) { return static_cast<CallFrame*>(callFrameBase); }
        Register* registers() { return this; }

        CallFrame& operator=(const Register& r) { *static_cast<Register*>(this) = r; return *this; }

        CallFrame* callerFrame() const { return this[JSStack::CallerFrame].callFrame(); }
#if ENABLE(JIT) || ENABLE(LLINT)
        ReturnAddressPtr returnPC() const { return ReturnAddressPtr(this[JSStack::ReturnPC].vPC()); }
        bool hasReturnPC() const { return !!this[JSStack::ReturnPC].vPC(); }
        void clearReturnPC() { registers()[JSStack::ReturnPC] = static_cast<Instruction*>(0); }
#endif
        AbstractPC abstractReturnPC(JSGlobalData& globalData) { return AbstractPC(globalData, this); }
#if USE(JSVALUE32_64)
        unsigned bytecodeOffsetForNonDFGCode() const;
        void setBytecodeOffsetForNonDFGCode(unsigned offset);
#else
        unsigned bytecodeOffsetForNonDFGCode() const
        {
            ASSERT(codeBlock());
            return this[JSStack::ArgumentCount].tag();
        }
        
        void setBytecodeOffsetForNonDFGCode(unsigned offset)
        {
            ASSERT(codeBlock());
            this[JSStack::ArgumentCount].tag() = static_cast<int32_t>(offset);
        }
#endif

        Register* frameExtent()
        {
            if (!codeBlock())
                return registers();
            return frameExtentInternal();
        }
    
        Register* frameExtentInternal();
    
#if ENABLE(DFG_JIT)
        InlineCallFrame* inlineCallFrame() const { return this[JSStack::ReturnPC].asInlineCallFrame(); }
        unsigned codeOriginIndexForDFG() const { return this[JSStack::ArgumentCount].tag(); }
#else
        // This will never be called if !ENABLE(DFG_JIT) since all calls should be guarded by
        // isInlineCallFrame(). But to make it easier to write code without having a bunch of
        // #if's, we make a dummy implementation available anyway.
        InlineCallFrame* inlineCallFrame() const
        {
            RELEASE_ASSERT_NOT_REACHED();
            return 0;
        }
#endif
#if USE(JSVALUE32_64)
        Instruction* currentVPC() const
        {
            return bitwise_cast<Instruction*>(this[JSStack::ArgumentCount].tag());
        }
        void setCurrentVPC(Instruction* vpc)
        {
            this[JSStack::ArgumentCount].tag() = bitwise_cast<int32_t>(vpc);
        }
#else
        Instruction* currentVPC() const;
        void setCurrentVPC(Instruction* vpc);
#endif

        void setCallerFrame(CallFrame* callerFrame) { static_cast<Register*>(this)[JSStack::CallerFrame] = callerFrame; }
        void setScope(JSScope* scope) { static_cast<Register*>(this)[JSStack::ScopeChain] = scope; }

        ALWAYS_INLINE void init(CodeBlock* codeBlock, Instruction* vPC, JSScope* scope,
            CallFrame* callerFrame, int argc, JSObject* callee)
        {
            ASSERT(callerFrame); // Use noCaller() rather than 0 for the outer host call frame caller.
            ASSERT(callerFrame == noCaller() || callerFrame->removeHostCallFrameFlag()->stack()->end() >= this);

            setCodeBlock(codeBlock);
            setScope(scope);
            setCallerFrame(callerFrame);
            setReturnPC(vPC); // This is either an Instruction* or a pointer into JIT generated code stored as an Instruction*.
            setArgumentCountIncludingThis(argc); // original argument count (for the sake of the "arguments" object)
            setCallee(callee);
        }

        // Read a register from the codeframe (or constant from the CodeBlock).
        Register& r(int);
        // Read a register for a non-constant 
        Register& uncheckedR(int);

        // Access to arguments as passed. (After capture, arguments may move to a different location.)
        size_t argumentCount() const { return argumentCountIncludingThis() - 1; }
        size_t argumentCountIncludingThis() const { return this[JSStack::ArgumentCount].payload(); }
        static int argumentOffset(int argument) { return s_firstArgumentOffset - argument; }
        static int argumentOffsetIncludingThis(int argument) { return s_thisArgumentOffset - argument; }

        // In the following (argument() and setArgument()), the 'argument'
        // parameter is the index of the arguments of the target function of
        // this frame. The index starts at 0 for the first arg, 1 for the
        // second, etc.
        //
        // The arguments (in this case) do not include the 'this' value.
        // arguments(0) will not fetch the 'this' value. To get/set 'this',
        // use thisValue() and setThisValue() below.

        JSValue argument(size_t argument)
        {
            if (argument >= argumentCount())
                 return jsUndefined();
            return this[argumentOffset(argument)].jsValue();
        }
        void setArgument(size_t argument, JSValue value)
        {
            this[argumentOffset(argument)] = value;
        }

        static int thisArgumentOffset() { return argumentOffsetIncludingThis(0); }
        JSValue thisValue() { return this[thisArgumentOffset()].jsValue(); }
        void setThisValue(JSValue value) { this[thisArgumentOffset()] = value; }

        JSValue argumentAfterCapture(size_t argument);

        static int offsetFor(size_t argumentCountIncludingThis) { return argumentCountIncludingThis + JSStack::CallFrameHeaderSize; }

        // FIXME: Remove these.
        int hostThisRegister() { return thisArgumentOffset(); }
        JSValue hostThisValue() { return thisValue(); }

        static CallFrame* noCaller() { return reinterpret_cast<CallFrame*>(HostCallFrameFlag); }

        bool hasHostCallFrameFlag() const { return reinterpret_cast<intptr_t>(this) & HostCallFrameFlag; }
        CallFrame* addHostCallFrameFlag() const { return reinterpret_cast<CallFrame*>(reinterpret_cast<intptr_t>(this) | HostCallFrameFlag); }
        CallFrame* removeHostCallFrameFlag() { return reinterpret_cast<CallFrame*>(reinterpret_cast<intptr_t>(this) & ~HostCallFrameFlag); }

        void setArgumentCountIncludingThis(int count) { static_cast<Register*>(this)[JSStack::ArgumentCount].payload() = count; }
        void setCallee(JSObject* callee) { static_cast<Register*>(this)[JSStack::Callee] = Register::withCallee(callee); }
        void setCodeBlock(CodeBlock* codeBlock) { static_cast<Register*>(this)[JSStack::CodeBlock] = codeBlock; }
        void setReturnPC(void* value) { static_cast<Register*>(this)[JSStack::ReturnPC] = (Instruction*)value; }
        
#if ENABLE(DFG_JIT)
        bool isInlineCallFrame();
        
        void setInlineCallFrame(InlineCallFrame* inlineCallFrame) { static_cast<Register*>(this)[JSStack::ReturnPC] = inlineCallFrame; }
        
        // Call this to get the semantically correct JS CallFrame* for the
        // currently executing function.
        CallFrame* trueCallFrame(AbstractPC);
        
        // Call this to get the semantically correct JS CallFrame* corresponding
        // to the caller. This resolves issues surrounding inlining and the
        // HostCallFrameFlag stuff.
        CallFrame* trueCallerFrame();
        
        CodeBlock* someCodeBlockForPossiblyInlinedCode();
#else
        bool isInlineCallFrame() { return false; }
        
        CallFrame* trueCallFrame(AbstractPC) { return this; }
        CallFrame* trueCallerFrame() { return callerFrame()->removeHostCallFrameFlag(); }
        
        CodeBlock* someCodeBlockForPossiblyInlinedCode() { return codeBlock(); }
#endif
        CallFrame* callerFrameNoFlags() { return callerFrame()->removeHostCallFrameFlag(); }
        
        // Call this to get the true call frame (accounted for inlining and any
        // other optimizations), when you have entered into VM code through one
        // of the "blessed" entrypoints (JITStubs or DFGOperations). This means
        // that if you're pretty much anywhere in the VM you can safely call this;
        // though if you were to magically get an ExecState* by, say, interrupting
        // a thread that is running JS code and brutishly scraped the call frame
        // register, calling this method would probably lead to horrible things
        // happening.
        CallFrame* trueCallFrameFromVMCode() { return trueCallFrame(AbstractPC()); }

    private:
        static const intptr_t HostCallFrameFlag = 1;
        static const int s_thisArgumentOffset = -1 - JSStack::CallFrameHeaderSize;
        static const int s_firstArgumentOffset = s_thisArgumentOffset - 1;

#ifndef NDEBUG
        JSStack* stack();
#endif
#if ENABLE(DFG_JIT)
        bool isInlineCallFrameSlow();
#endif
        ExecState();
        ~ExecState();

        // The following are for internal use in debugging and verification
        // code only and not meant as an API for general usage:

        size_t argIndexForRegister(Register* reg)
        {
            // The register at 'offset' number of slots from the frame pointer
            // i.e.
            //       reg = frame[offset];
            //   ==> reg = frame + offset;
            //   ==> offset = reg - frame;
            int offset = reg - this->registers();

            // The offset is defined (based on argumentOffset()) to be:
            //       offset = s_firstArgumentOffset - argIndex;
            // Hence:
            //       argIndex = s_firstArgumentOffset - offset;
            size_t argIndex = s_firstArgumentOffset - offset;
            return argIndex;
        }

        JSValue getArgumentUnsafe(size_t argIndex)
        {
            // User beware! This method does not verify that there is a valid
            // argument at the specified argIndex. This is used for debugging
            // and verification code only. The caller is expected to know what
            // he/she is doing when calling this method.
            return this[argumentOffset(argIndex)].jsValue();
        }

        friend class JSStack;
        friend class VMInspector;
    };

} // namespace JSC

#endif // CallFrame_h
