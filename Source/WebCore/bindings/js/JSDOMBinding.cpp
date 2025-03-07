/*
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Apple Inc. All rights reserved.
 *  Copyright (C) 2007 Samuel Weinig <sam@webkit.org>
 *  Copyright (C) 2013 Michael Pruett <michael@68k.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "JSDOMBinding.h"

#include "BindingSecurity.h"
#include "CachedScript.h"
#include "DOMObjectHashTableMap.h"
#include "DOMStringList.h"
#include "ExceptionCode.h"
#include "ExceptionHeaders.h"
#include "ExceptionInterfaces.h"
#include "Frame.h"
#include "JSDOMWindowCustom.h"
#include "JSExceptionBase.h"
#include "ScriptCallStack.h"
#include <interpreter/Interpreter.h>
#include <runtime/DateInstance.h>
#include <runtime/Error.h>
#include <runtime/ExceptionHelpers.h>
#include <runtime/JSFunction.h>

using namespace JSC;

namespace WebCore {

ASSERT_HAS_TRIVIAL_DESTRUCTOR(DOMConstructorObject);
ASSERT_HAS_TRIVIAL_DESTRUCTOR(DOMConstructorWithDocument);

const JSC::HashTable* getHashTableForGlobalData(JSGlobalData& globalData, const JSC::HashTable* staticTable)
{
    return DOMObjectHashTableMap::mapFor(globalData).get(staticTable);
}

JSValue jsStringOrNull(ExecState* exec, const String& s)
{
    if (s.isNull())
        return jsNull();
    return jsStringWithCache(exec, s);
}

JSValue jsOwnedStringOrNull(ExecState* exec, const String& s)
{
    if (s.isNull())
        return jsNull();
    return jsOwnedString(exec, s);
}

JSValue jsStringOrUndefined(ExecState* exec, const String& s)
{
    if (s.isNull())
        return jsUndefined();
    return jsStringWithCache(exec, s);
}

JSValue jsString(ExecState* exec, const KURL& url)
{
    return jsStringWithCache(exec, url.string());
}

JSValue jsStringOrNull(ExecState* exec, const KURL& url)
{
    if (url.isNull())
        return jsNull();
    return jsStringWithCache(exec, url.string());
}

JSValue jsStringOrUndefined(ExecState* exec, const KURL& url)
{
    if (url.isNull())
        return jsUndefined();
    return jsStringWithCache(exec, url.string());
}

AtomicStringImpl* findAtomicString(PropertyName propertyName)
{
    StringImpl* impl = propertyName.publicName();
    if (!impl)
        return 0;
    ASSERT(impl->existingHash());
    return AtomicString::find(impl);
}

String valueToStringWithNullCheck(ExecState* exec, JSValue value)
{
    if (value.isNull())
        return String();
    return value.toString(exec)->value(exec);
}

String valueToStringWithUndefinedOrNullCheck(ExecState* exec, JSValue value)
{
    if (value.isUndefinedOrNull())
        return String();
    return value.toString(exec)->value(exec);
}

JSValue jsDateOrNull(ExecState* exec, double value)
{
    if (!std::isfinite(value))
        return jsNull();
    return DateInstance::create(exec, exec->lexicalGlobalObject()->dateStructure(), value);
}

double valueToDate(ExecState* exec, JSValue value)
{
    if (value.isNumber())
        return value.asNumber();
    if (!value.inherits(&DateInstance::s_info))
        return std::numeric_limits<double>::quiet_NaN();
    return static_cast<DateInstance*>(value.toObject(exec))->internalNumber();
}

JSC::JSValue jsArray(JSC::ExecState* exec, JSDOMGlobalObject* globalObject, PassRefPtr<DOMStringList> stringList)
{
    JSC::MarkedArgumentBuffer list;
    if (stringList) {
        for (unsigned i = 0; i < stringList->length(); ++i)
            list.append(jsStringWithCache(exec, stringList->item(i)));
    }
    return JSC::constructArray(exec, 0, globalObject, list);
}

void reportException(ExecState* exec, JSValue exception, CachedScript* cachedScript)
{
    if (isTerminatedExecutionException(exception))
        return;

    Interpreter::ErrorHandlingMode mode(exec);
    String errorMessage = exception.toString(exec)->value(exec);
    int lineNumber = 0;
    String exceptionSourceURL;

    RefCountedArray<StackFrame> stackTrace = exec->globalData().exceptionStack;
    exec->clearException();
    exec->clearSupplementaryExceptionInfo();

    if (exception.isObject()) {
        JSObject* exceptionObject = exception.toObject(exec);
        lineNumber = exceptionObject->get(exec, Identifier(exec, "line")).toInt32(exec);
        exceptionSourceURL = exceptionObject->get(exec, Identifier(exec, "sourceURL")).toString(exec)->value(exec);
    } else if (stackTrace.size()) {
        lineNumber = stackTrace[0].line();
        exceptionSourceURL = stackTrace[0].friendlySourceURL();
    }

    if (ExceptionBase* exceptionBase = toExceptionBase(exception))
        errorMessage = exceptionBase->message() + ": "  + exceptionBase->description();

    JSDOMGlobalObject* globalObject = jsCast<JSDOMGlobalObject*>(exec->lexicalGlobalObject());
    if (JSDOMWindow* window = jsDynamicCast<JSDOMWindow*>(globalObject)) {
        if (!window->impl()->isCurrentlyDisplayedInFrame())
            return;
    }
    ScriptExecutionContext* scriptExecutionContext = globalObject->scriptExecutionContext();
    scriptExecutionContext->reportException(errorMessage, lineNumber, exceptionSourceURL, 0, cachedScript);
}

void reportCurrentException(ExecState* exec)
{
    JSValue exception = exec->exception();
    exec->clearException();
    reportException(exec, exception);
}

#define TRY_TO_CREATE_EXCEPTION(interfaceName) \
    case interfaceName##Type: \
        errorObject = toJS(exec, globalObject, interfaceName::create(description)); \
        break;

void setDOMException(ExecState* exec, ExceptionCode ec)
{
    if (!ec || exec->hadException())
        return;

    // FIXME: Handle other WebIDL exception types.
    if (ec == TypeError) {
        throwTypeError(exec);
        return;
    }

    // FIXME: All callers to setDOMException need to pass in the right global object
    // for now, we're going to assume the lexicalGlobalObject.  Which is wrong in cases like this:
    // frames[0].document.createElement(null, null); // throws an exception which should have the subframes prototypes.
    JSDOMGlobalObject* globalObject = deprecatedGlobalObjectForPrototype(exec);

    ExceptionCodeDescription description(ec);

    JSValue errorObject;
    switch (description.type) {
        DOM_EXCEPTION_INTERFACES_FOR_EACH(TRY_TO_CREATE_EXCEPTION)
    }

    ASSERT(errorObject);
    throwError(exec, errorObject);
}

#undef TRY_TO_CREATE_EXCEPTION

bool shouldAllowAccessToNode(ExecState* exec, Node* node)
{
    return BindingSecurity::shouldAllowAccessToNode(exec, node);
}

bool shouldAllowAccessToFrame(ExecState* exec, Frame* target)
{
    return BindingSecurity::shouldAllowAccessToFrame(exec, target);
}

bool shouldAllowAccessToFrame(ExecState* exec, Frame* frame, String& message)
{
    if (!frame)
        return false;
    if (BindingSecurity::shouldAllowAccessToFrame(exec, frame, DoNotReportSecurityError))
        return true;
    message = frame->document()->domWindow()->crossDomainAccessErrorMessage(activeDOMWindow(exec));
    return false;
}

bool shouldAllowAccessToDOMWindow(ExecState* exec, DOMWindow* target, String& message)
{
    if (!target)
        return false;
    if (BindingSecurity::shouldAllowAccessToDOMWindow(exec, target, DoNotReportSecurityError))
        return true;
    message = target->crossDomainAccessErrorMessage(activeDOMWindow(exec));
    return false;
}

void printErrorMessageForFrame(Frame* frame, const String& message)
{
    if (!frame)
        return;
    frame->document()->domWindow()->printErrorMessage(message);
}

JSValue objectToStringFunctionGetter(ExecState* exec, JSValue, PropertyName propertyName)
{
    return JSFunction::create(exec, exec->lexicalGlobalObject(), 0, propertyName.publicName(), objectProtoFuncToString);
}

Structure* getCachedDOMStructure(JSDOMGlobalObject* globalObject, const ClassInfo* classInfo)
{
    JSDOMStructureMap& structures = globalObject->structures();
    return structures.get(classInfo).get();
}

Structure* cacheDOMStructure(JSDOMGlobalObject* globalObject, Structure* structure, const ClassInfo* classInfo)
{
    JSDOMStructureMap& structures = globalObject->structures();
    ASSERT(!structures.contains(classInfo));
    return structures.set(classInfo, WriteBarrier<Structure>(globalObject->globalData(), globalObject, structure)).iterator->value.get();
}

static const int32_t kMaxInt32 = 0x7fffffff;
static const int32_t kMinInt32 = -kMaxInt32 - 1;
static const uint32_t kMaxUInt32 = 0xffffffffU;
static const int64_t kJSMaxInteger = 0x20000000000000LL - 1; // 2^53 - 1, largest integer exactly representable in ECMAScript.

static double enforceRange(ExecState* exec, double x, double minimum, double maximum)
{
    if (std::isnan(x) || std::isinf(x)) {
        throwTypeError(exec);
        return 0;
    }
    x = trunc(x);
    if (x < minimum || x > maximum) {
        throwTypeError(exec);
        return 0;
    }
    return x;
}

// http://www.w3.org/TR/WebIDL/#es-long
int32_t toInt32EnforceRange(ExecState* exec, JSValue value)
{
    if (value.isInt32())
        return value.toInt32(exec);

    double x = value.toNumber(exec);
    if (exec->hadException())
        return 0;
    return enforceRange(exec, x, kMinInt32, kMaxInt32);
}

// http://www.w3.org/TR/WebIDL/#es-unsigned-long
uint32_t toUInt32EnforceRange(ExecState* exec, JSValue value)
{
    if (value.isUInt32())
        return value.toUInt32(exec);

    double x = value.toNumber(exec);
    if (exec->hadException())
        return 0;
    return enforceRange(exec, x, 0, kMaxUInt32);
}

// http://www.w3.org/TR/WebIDL/#es-long-long
int64_t toInt64(ExecState* exec, JSValue value, IntegerConversionConfiguration configuration)
{
    if (value.isInt32())
        return value.toInt32(exec);

    double x = value.toNumber(exec);
    if (exec->hadException())
        return 0;

    if (configuration == EnforceRange)
        return enforceRange(exec, x, -kJSMaxInteger, kJSMaxInteger);

    // Map NaNs and +/-Infinity to 0; convert finite values modulo 2^64.
    unsigned long long n;
    doubleToInteger(x, n);
    return n;
}

// http://www.w3.org/TR/WebIDL/#es-unsigned-long-long
uint64_t toUInt64(ExecState* exec, JSValue value, IntegerConversionConfiguration configuration)
{
    if (value.isUInt32())
        return value.toUInt32(exec);

    double x = value.toNumber(exec);
    if (exec->hadException())
        return 0;

    if (configuration == EnforceRange)
        return enforceRange(exec, x, 0, kJSMaxInteger);

    // Map NaNs and +/-Infinity to 0; convert finite values modulo 2^64.
    unsigned long long n;
    doubleToInteger(x, n);
    return n;
}

} // namespace WebCore
