// Copyright (c) 2015 baidu-rpc authors.
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

#ifndef BRPC_SHARED_OBJECT_H
#define BRPC_SHARED_OBJECT_H

#include "base/intrusive_ptr.hpp"                   // base::intrusive_ptr
#include "base/atomicops.h"


namespace brpc {

// Inherit this class to be intrusively shared. Comparing to shared_ptr,
// intrusive_ptr saves one malloc (for shared_count) and gets better cache
// locality when the ref/deref are frequent, in the cost of inability of
// weak_ptr and worse interfacing.
class SharedObject {
friend void intrusive_ptr_add_ref(SharedObject*);
friend void intrusive_ptr_release(SharedObject*);

public:
    SharedObject() : _nref(0) { }
    int ref_count() const { return _nref.load(base::memory_order_relaxed); }
    
    // Add ref and returns the ref_count seen before added.
    // The effect is basically same as base::intrusive_ptr<T>(obj).detach()
    // except that the latter one does not return the seen ref_count which is
    // useful in some scenarios.
    int AddRefManually()
    { return _nref.fetch_add(1, base::memory_order_relaxed); }

    // Remove one ref, if the ref_count hit zero, delete this object.
    // Same as base::intrusive_ptr<T>(obj, false).reset(NULL)
    void RemoveRefManually() {
        if (_nref.fetch_sub(1, base::memory_order_release) == 1) {
            base::atomic_thread_fence(base::memory_order_acquire);
            delete this;
        }
    }

protected:
    virtual ~SharedObject() { }
private:
    base::atomic<int> _nref;
};

inline void intrusive_ptr_add_ref(SharedObject* obj) {
    obj->AddRefManually();
}

inline void intrusive_ptr_release(SharedObject* obj) {
    obj->RemoveRefManually();
}

} // namespace brpc


#endif // BRPC_SHARED_OBJECT_H

