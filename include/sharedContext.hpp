#ifndef SHARED_CONTEXT_H
#define SHARED_CONTEXT_H

class PrivateNetworkAccess;

struct SharedContext 
{
    SharedContext():
        mPrivateNetworkAccess(nullptr)
    {}

    PrivateNetworkAccess* mPrivateNetworkAccess;
};

#endif