//
// Created by pkanev on 5/10/2017.
//

#include <sstream>
#include <ArgConverter.h>
#include <NativeScriptAssert.h>
#include <v8_inspector/third_party/inspector_protocol/crdtp/json.h>
#include "DOMDomainCallbackHandlers.h"

using namespace tns;

void DOMDomainCallbackHandlers::DocumentUpdatedCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto domAgentInstance = V8DOMAgentImpl::Instance;

    if (!domAgentInstance) {
        return;
    }

    domAgentInstance->m_frontend.documentUpdated();
}

void DOMDomainCallbackHandlers::ChildNodeInsertedCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    try {
        auto domAgentInstance = V8DOMAgentImpl::Instance;

        if (!domAgentInstance) {
            return;
        }

        auto isolate = args.GetIsolate();

        v8::HandleScope scope(isolate);

        if (args.Length() != 3 || !(args[0]->IsNumber() && args[1]->IsNumber() && args[2]->IsString())) {
            throw NativeScriptException("Calling ChildNodeInserted with invalid arguments. Required params: parentId: number, lastId: number, node: JSON String");
        }

        auto context = isolate->GetCurrentContext();
        auto parentId = args[0]->ToNumber(context).ToLocalChecked();
        auto lastId = args[1]->ToNumber(context).ToLocalChecked();
        auto node = args[2]->ToString(context).ToLocalChecked();

        auto resultString = V8DOMAgentImpl::AddBackendNodeIdProperty(isolate, node);
        auto nodeUtf16Data = resultString.data();
        const String16& nodeString16 = String16((const uint16_t*) nodeUtf16Data);
        std::vector<uint8_t> cbor;
        v8_crdtp::json::ConvertJSONToCBOR(v8_crdtp::span<uint16_t>(nodeString16.characters16(), nodeString16.length()), &cbor);
        std::unique_ptr<protocol::Value> protocolNodeJson = protocol::Value::parseBinary(cbor.data(), cbor.size());

        protocol::ErrorSupport errorSupport;
        auto domNode = protocol::DOM::Node::fromValue(protocolNodeJson.get(), &errorSupport);

        std::vector<uint8_t> json;
        v8_crdtp::json::ConvertCBORToJSON(errorSupport.Errors(), &json);
        auto errorSupportString = String16(reinterpret_cast<const char*>(json.data()), json.size()).utf8();
        if (!errorSupportString.empty()) {
            auto errorMessage = "Error while parsing debug `DOM Node` object. ";
            DEBUG_WRITE_FORCE("%s Error: %s", errorMessage, errorSupportString.c_str());
            return;
        }

        domAgentInstance->m_frontend.childNodeInserted(parentId->Int32Value(context).ToChecked(), lastId->Int32Value(context).ToChecked(), std::move(domNode));
    } catch (NativeScriptException& e) {
        e.ReThrowToV8();
    } catch (std::exception e) {
        std::stringstream ss;
        ss << "Error: c exception: " << e.what() << std::endl;
        NativeScriptException nsEx(ss.str());
        nsEx.ReThrowToV8();
    } catch (...) {
        NativeScriptException nsEx(std::string("Error: c exception!"));
        nsEx.ReThrowToV8();
    }
}

void DOMDomainCallbackHandlers::ChildNodeRemovedCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    try {
        auto domAgentInstance = V8DOMAgentImpl::Instance;

        if (!domAgentInstance) {
            return;
        }

        auto isolate = args.GetIsolate();

        v8::HandleScope scope(isolate);

        if (args.Length() != 2 || !(args[0]->IsNumber() && args[1]->IsNumber())) {
            throw NativeScriptException("Calling ChildNodeRemoved with invalid arguments. Required params: parentId: number, nodeId: number");
        }

        auto context = isolate->GetCurrentContext();
        auto parentId = args[0]->ToNumber(context).ToLocalChecked();
        auto nodeId = args[1]->ToNumber(context).ToLocalChecked();

        domAgentInstance->m_frontend.childNodeRemoved(parentId->Int32Value(context).ToChecked(), nodeId->Int32Value(context).ToChecked());
    } catch (NativeScriptException& e) {
        e.ReThrowToV8();
    } catch (std::exception e) {
        std::stringstream ss;
        ss << "Error: c exception: " << e.what() << std::endl;
        NativeScriptException nsEx(ss.str());
        nsEx.ReThrowToV8();
    } catch (...) {
        NativeScriptException nsEx(std::string("Error: c exception!"));
        nsEx.ReThrowToV8();
    }
}

void DOMDomainCallbackHandlers::AttributeModifiedCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    try {
        auto domAgentInstance = V8DOMAgentImpl::Instance;

        if (!domAgentInstance) {
            return;
        }

        auto isolate = args.GetIsolate();

        v8::HandleScope scope(isolate);

        if (args.Length() != 3 || !(args[0]->IsNumber() && args[1]->IsString() && args[2]->IsString())) {
            throw NativeScriptException("Calling AttributeModified with invalid arguments. Required params: nodeId: number, name: string, value: string");
        }

        auto context = isolate->GetCurrentContext();
        auto nodeId = args[0]->ToNumber(context).ToLocalChecked();
        auto attributeName = args[1]->ToString(context).ToLocalChecked();
        auto attributeValue = args[2]->ToString(context).ToLocalChecked();

        domAgentInstance->m_frontend.attributeModified(nodeId->Int32Value(context).ToChecked(),
                v8_inspector::toProtocolString(isolate, attributeName),
                v8_inspector::toProtocolString(isolate, attributeValue));
    } catch (NativeScriptException& e) {
        e.ReThrowToV8();
    } catch (std::exception e) {
        std::stringstream ss;
        ss << "Error: c exception: " << e.what() << std::endl;
        NativeScriptException nsEx(ss.str());
        nsEx.ReThrowToV8();
    } catch (...) {
        NativeScriptException nsEx(std::string("Error: c exception!"));
        nsEx.ReThrowToV8();
    }
}

void DOMDomainCallbackHandlers::AttributeRemovedCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    try {
        auto domAgentInstance = V8DOMAgentImpl::Instance;

        if (!domAgentInstance) {
            return;
        }
        auto isolate = args.GetIsolate();

        v8::HandleScope scope(isolate);

        if (args.Length() != 2 || !(args[0]->IsNumber() && args[1]->IsString())) {
            throw NativeScriptException("Calling AttributeRemoved with invalid arguments. Required params: nodeId: number, name: string");
        }

        auto context = isolate->GetCurrentContext();
        auto nodeId = args[0]->ToNumber(context).ToLocalChecked();
        auto attributeName = args[1]->ToString(context).ToLocalChecked();

        domAgentInstance->m_frontend.attributeRemoved(nodeId->Int32Value(context).ToChecked(),
                v8_inspector::toProtocolString(isolate, attributeName));
    } catch (NativeScriptException& e) {
        e.ReThrowToV8();
    } catch (std::exception e) {
        std::stringstream ss;
        ss << "Error: c exception: " << e.what() << std::endl;
        NativeScriptException nsEx(ss.str());
        nsEx.ReThrowToV8();
    } catch (...) {
        NativeScriptException nsEx(std::string("Error: c exception!"));
        nsEx.ReThrowToV8();
    }
}
