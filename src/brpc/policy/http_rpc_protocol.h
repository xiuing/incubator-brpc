// Copyright (c) 2014 baidu-rpc authors.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef BRPC_POLICY_HTTP_RPC_PROTOCOL_H
#define BRPC_POLICY_HTTP_RPC_PROTOCOL_H

#include "brpc/details/http_message.h"         // HttpMessage
#include "brpc/details/controller_private_accessor.h"
#include "brpc/input_messenger.h"              // InputMessenger
#include "brpc/protocol.h"

namespace brpc {
namespace policy {

// Put commonly used std::strings (or other constants that need memory
// allocations) in this struct to avoid memory allocations for each request.
struct CommonStrings {
    std::string ACCEPT;
    std::string DEFAULT_ACCEPT;
    std::string USER_AGENT;
    std::string DEFAULT_USER_AGENT;
    std::string CONTENT_TYPE;
    std::string CONTENT_TYPE_TEXT;
    std::string CONTENT_TYPE_JSON;
    std::string CONTENT_TYPE_PROTO;
    std::string ERROR_CODE;
    std::string AUTHORIZATION;
    std::string ACCEPT_ENCODING;
    std::string CONTENT_ENCODING;
    std::string CONTENT_LENGTH;
    std::string GZIP;
    std::string CONNECTION;
    std::string KEEP_ALIVE;
    std::string CLOSE;
    // Many users already GetHeader("log-id") in their code, it's difficult to
    // rename this to `x-bd-log-id'.
    // NOTE: Keep in mind that this name also appears inside `http_message.cpp'
    std::string LOG_ID;
    std::string DEFAULT_METHOD;
    std::string NO_METHOD;
    std::string H2_SCHEME;
    std::string H2_SCHEME_HTTP;
    std::string H2_SCHEME_HTTPS;
    std::string H2_AUTHORITY;
    std::string H2_PATH;
    std::string H2_STATUS;
    std::string STATUS_200;
    std::string H2_METHOD;
    std::string METHOD_GET;
    std::string METHOD_POST;

    CommonStrings();
};

// Used in UT.
class HttpContext : public ReadableProgressiveAttachment
                       , public InputMessageBase
                       , public HttpMessage {
public:
    HttpContext(bool read_body_progressively = false)
        : InputMessageBase()
        , HttpMessage(read_body_progressively)
        , _is_stage2(false) {
        // add one ref for Destroy
        base::intrusive_ptr<HttpContext>(this).detach();
    }

    void AddOneRefForStage2() {
        base::intrusive_ptr<HttpContext>(this).detach();
        _is_stage2 = true;
    }

    void RemoveOneRefForStage2() {
        base::intrusive_ptr<HttpContext>(this, false);
    }

    // True if AddOneRefForStage2() was ever called.
    bool is_stage2() const { return _is_stage2; }

    // @InputMessageBase
    void DestroyImpl() {
        RemoveOneRefForStage2();
    }

    // @ReadableProgressiveAttachment
    void ReadProgressiveAttachmentBy(ProgressiveReader* r) {
        return SetBodyReader(r);
    }

private:
    bool _is_stage2;
};

// Implement functions required in protocol.h
ParseResult ParseHttpMessage(base::IOBuf *source, Socket *socket,
                             bool read_eof, const void *arg);
void ProcessHttpRequest(InputMessageBase *msg);
void ProcessHttpResponse(InputMessageBase* msg);
bool VerifyHttpRequest(const InputMessageBase* msg);
void SerializeHttpRequest(base::IOBuf* request_buf,
                          Controller* cntl,
                          const google::protobuf::Message* msg);
void PackHttpRequest(base::IOBuf* buf,
                     SocketMessage** user_message_out,
                     uint64_t correlation_id,
                     const google::protobuf::MethodDescriptor* method,
                     Controller* controller,
                     const base::IOBuf& request,
                     const Authenticator* auth);
bool ParseHttpServerAddress(base::EndPoint* out, const char* server_addr_and_port);
const std::string& GetHttpMethodName(const google::protobuf::MethodDescriptor*,
                                     const Controller*);

}  // namespace policy
} // namespace brpc


#endif // BRPC_POLICY_HTTP_RPC_PROTOCOL_H
