/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
 * Copyright (C) 2009, 2011 Google Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 *
 */

#include "config.h"

#if ENABLE(WORKERS)

#include "WorkerContext.h"

#include "ActiveDOMObject.h"
#include "ContentSecurityPolicy.h"
#include "DOMTimer.h"
#include "DOMURL.h"
#include "DOMWindow.h"
#include "ErrorEvent.h"
#include "Event.h"
#include "EventException.h"
#include "InspectorConsoleInstrumentation.h"
#include "KURL.h"
#include "MessagePort.h"
#include "NotImplemented.h"
#include "ScheduledAction.h"
#include "ScriptCallStack.h"
#include "ScriptSourceCode.h"
#include "ScriptValue.h"
#include "SecurityOrigin.h"
#include "WorkerInspectorController.h"
#include "WorkerLocation.h"
#include "WorkerNavigator.h"
#include "WorkerObjectProxy.h"
#include "WorkerScriptLoader.h"
#include "WorkerThread.h"
#include "WorkerThreadableLoader.h"
#include "XMLHttpRequestException.h"
#include <wtf/RefPtr.h>
#include <wtf/UnusedParam.h>

#if ENABLE(NOTIFICATIONS) || ENABLE(LEGACY_NOTIFICATIONS)
#include "NotificationCenter.h"
#endif

#include "ExceptionCode.h"

namespace WebCore {

class CloseWorkerContextTask : public ScriptExecutionContext::Task {
public:
    static PassOwnPtr<CloseWorkerContextTask> create()
    {
        return adoptPtr(new CloseWorkerContextTask);
    }

    virtual void performTask(ScriptExecutionContext *context)
    {
        ASSERT_WITH_SECURITY_IMPLICATION(context->isWorkerContext());
        WorkerContext* workerContext = static_cast<WorkerContext*>(context);
        // Notify parent that this context is closed. Parent is responsible for calling WorkerThread::stop().
        workerContext->thread()->workerReportingProxy().workerContextClosed();
    }

    virtual bool isCleanupTask() const { return true; }
};

WorkerContext::WorkerContext(const KURL& url, const String& userAgent, PassOwnPtr<GroupSettings> settings, WorkerThread* thread, PassRefPtr<SecurityOrigin> topOrigin)
    : m_url(url)
    , m_userAgent(userAgent)
    , m_groupSettings(settings)
    , m_script(adoptPtr(new WorkerScriptController(this)))
    , m_thread(thread)
#if ENABLE(INSPECTOR)
    , m_workerInspectorController(adoptPtr(new WorkerInspectorController(this)))
#endif
    , m_closing(false)
    , m_eventQueue(WorkerEventQueue::create(this))
    , m_topOrigin(topOrigin)
{
    setSecurityOrigin(SecurityOrigin::create(url));
}

WorkerContext::~WorkerContext()
{
    ASSERT(currentThread() == thread()->threadID());

    // Make sure we have no observers.
    notifyObserversOfStop();

    // Notify proxy that we are going away. This can free the WorkerThread object, so do not access it after this.
    thread()->workerReportingProxy().workerContextDestroyed();
}

void WorkerContext::applyContentSecurityPolicyFromString(const String& policy, ContentSecurityPolicy::HeaderType contentSecurityPolicyType)
{
    setContentSecurityPolicy(ContentSecurityPolicy::create(this));
    contentSecurityPolicy()->didReceiveHeader(policy, contentSecurityPolicyType);
}

ScriptExecutionContext* WorkerContext::scriptExecutionContext() const
{
    return const_cast<WorkerContext*>(this);
}

const KURL& WorkerContext::virtualURL() const
{
    return m_url;
}

KURL WorkerContext::virtualCompleteURL(const String& url) const
{
    return completeURL(url);
}

KURL WorkerContext::completeURL(const String& url) const
{
    // Always return a null URL when passed a null string.
    // FIXME: Should we change the KURL constructor to have this behavior?
    if (url.isNull())
        return KURL();
    // Always use UTF-8 in Workers.
    return KURL(m_url, url);
}

String WorkerContext::userAgent(const KURL&) const
{
    return m_userAgent;
}

void WorkerContext::disableEval(const String& errorMessage)
{
    m_script->disableEval(errorMessage);
}

WorkerLocation* WorkerContext::location() const
{
    if (!m_location)
        m_location = WorkerLocation::create(m_url);
    return m_location.get();
}

void WorkerContext::close()
{
    if (m_closing)
        return;

    // Let current script run to completion but prevent future script evaluations.
    // After m_closing is set, all the tasks in the queue continue to be fetched but only
    // tasks with isCleanupTask()==true will be executed.
    m_closing = true;
    postTask(CloseWorkerContextTask::create());
}

WorkerNavigator* WorkerContext::navigator() const
{
    if (!m_navigator)
        m_navigator = WorkerNavigator::create(m_userAgent);
    return m_navigator.get();
}

bool WorkerContext::hasPendingActivity() const
{
    ActiveDOMObjectsSet::const_iterator activeObjectsEnd = activeDOMObjects().end();
    for (ActiveDOMObjectsSet::const_iterator iter = activeDOMObjects().begin(); iter != activeObjectsEnd; ++iter) {
        if ((*iter)->hasPendingActivity())
            return true;
    }

    HashSet<MessagePort*>::const_iterator messagePortsEnd = messagePorts().end();
    for (HashSet<MessagePort*>::const_iterator iter = messagePorts().begin(); iter != messagePortsEnd; ++iter) {
        if ((*iter)->hasPendingActivity())
            return true;
    }

    return false;
}

void WorkerContext::postTask(PassOwnPtr<Task> task)
{
    thread()->runLoop().postTask(task);
}

int WorkerContext::setTimeout(PassOwnPtr<ScheduledAction> action, int timeout)
{
    return DOMTimer::install(scriptExecutionContext(), action, timeout, true);
}

void WorkerContext::clearTimeout(int timeoutId)
{
    DOMTimer::removeById(scriptExecutionContext(), timeoutId);
}

#if ENABLE(INSPECTOR)
void WorkerContext::clearInspector()
{
    m_workerInspectorController.clear();
}
#endif

int WorkerContext::setInterval(PassOwnPtr<ScheduledAction> action, int timeout)
{
    return DOMTimer::install(scriptExecutionContext(), action, timeout, false);
}

void WorkerContext::clearInterval(int timeoutId)
{
    DOMTimer::removeById(scriptExecutionContext(), timeoutId);
}

void WorkerContext::importScripts(const Vector<String>& urls, ExceptionCode& ec)
{
    ASSERT(contentSecurityPolicy());
    ec = 0;
    Vector<String>::const_iterator urlsEnd = urls.end();
    Vector<KURL> completedURLs;
    for (Vector<String>::const_iterator it = urls.begin(); it != urlsEnd; ++it) {
        const KURL& url = scriptExecutionContext()->completeURL(*it);
        if (!url.isValid()) {
            ec = SYNTAX_ERR;
            return;
        }
        completedURLs.append(url);
    }
    Vector<KURL>::const_iterator end = completedURLs.end();

    for (Vector<KURL>::const_iterator it = completedURLs.begin(); it != end; ++it) {
        RefPtr<WorkerScriptLoader> scriptLoader(WorkerScriptLoader::create());
#if PLATFORM(BLACKBERRY)
        scriptLoader->setTargetType(ResourceRequest::TargetIsScript);
#endif
        scriptLoader->loadSynchronously(scriptExecutionContext(), *it, AllowCrossOriginRequests);

        // If the fetching attempt failed, throw a NETWORK_ERR exception and abort all these steps.
        if (scriptLoader->failed()) {
            ec = XMLHttpRequestException::NETWORK_ERR;
            return;
        }

        InspectorInstrumentation::scriptImported(scriptExecutionContext(), scriptLoader->identifier(), scriptLoader->script());

        ScriptValue exception;
        m_script->evaluate(ScriptSourceCode(scriptLoader->script(), scriptLoader->responseURL()), &exception);
        if (!exception.hasNoValue()) {
            m_script->setException(exception);
            return;
        }
    }
}

EventTarget* WorkerContext::errorEventTarget()
{
    return this;
}

void WorkerContext::logExceptionToConsole(const String& errorMessage, const String& sourceURL, int lineNumber, PassRefPtr<ScriptCallStack>)
{
    thread()->workerReportingProxy().postExceptionToWorkerObject(errorMessage, lineNumber, sourceURL);
}

void WorkerContext::addConsoleMessage(MessageSource source, MessageLevel level, const String& message, unsigned long requestIdentifier)
{
    if (!isContextThread()) {
        postTask(AddConsoleMessageTask::create(source, level, message));
        return;
    }
    thread()->workerReportingProxy().postConsoleMessageToWorkerObject(source, level, message, 0, String());

    addMessageToWorkerConsole(source, level, message, String(), 0, 0, 0, requestIdentifier);
}

void WorkerContext::addMessage(MessageSource source, MessageLevel level, const String& message, const String& sourceURL, unsigned lineNumber, PassRefPtr<ScriptCallStack> callStack, ScriptState* state, unsigned long requestIdentifier)
{
    if (!isContextThread()) {
        postTask(AddConsoleMessageTask::create(source, level, message));
        return;
    }
    thread()->workerReportingProxy().postConsoleMessageToWorkerObject(source, level, message, lineNumber, sourceURL);
    addMessageToWorkerConsole(source, level, message, sourceURL, lineNumber, callStack, state, requestIdentifier);
}

void WorkerContext::addMessageToWorkerConsole(MessageSource source, MessageLevel level, const String& message, const String& sourceURL, unsigned lineNumber, PassRefPtr<ScriptCallStack> callStack, ScriptState* state, unsigned long requestIdentifier)
{
    ASSERT(isContextThread());
    if (callStack)
        InspectorInstrumentation::addMessageToConsole(this, source, LogMessageType, level, message, callStack, requestIdentifier);
    else
        InspectorInstrumentation::addMessageToConsole(this, source, LogMessageType, level, message, sourceURL, lineNumber, state, requestIdentifier);
}

bool WorkerContext::isContextThread() const
{
    return currentThread() == thread()->threadID();
}

bool WorkerContext::isJSExecutionForbidden() const
{
    return m_script->isExecutionForbidden();
}

EventTargetData* WorkerContext::eventTargetData()
{
    return &m_eventTargetData;
}

EventTargetData* WorkerContext::ensureEventTargetData()
{
    return &m_eventTargetData;
}

WorkerContext::Observer::Observer(WorkerContext* context)
    : m_context(context)
{
    ASSERT(m_context && m_context->isContextThread());
    m_context->registerObserver(this);
}

WorkerContext::Observer::~Observer()
{
    if (!m_context)
        return;
    ASSERT(m_context->isContextThread());
    m_context->unregisterObserver(this);
}

void WorkerContext::Observer::stopObserving()
{
    if (!m_context)
        return;
    ASSERT(m_context->isContextThread());
    m_context->unregisterObserver(this);
    m_context = 0;
}

void WorkerContext::registerObserver(Observer* observer)
{
    ASSERT(observer);
    m_workerObservers.add(observer);
}

void WorkerContext::unregisterObserver(Observer* observer)
{
    ASSERT(observer);
    m_workerObservers.remove(observer);
}

void WorkerContext::notifyObserversOfStop()
{
    HashSet<Observer*>::iterator iter = m_workerObservers.begin();
    while (iter != m_workerObservers.end()) {
        WorkerContext::Observer* observer = *iter;
        observer->stopObserving();
        observer->notifyStop();
        iter = m_workerObservers.begin();
    }
}

WorkerEventQueue* WorkerContext::eventQueue() const
{
    return m_eventQueue.get();
}

} // namespace WebCore

#endif // ENABLE(WORKERS)
