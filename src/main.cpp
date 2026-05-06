// main.cpp — LeviLamina plugin entry point for multiversion bridge

#include <ll/api/plugin/NativePlugin.h>
#include <ll/api/plugin/RegisterHelper.h>

// Forward declarations from PacketHooks.cpp
void registerPacketHooks(ll::plugin::NativePlugin& plugin);
void registerDisconnectHook();

namespace multiversion {

class MultiversionPlugin {
public:
    static MultiversionPlugin& getInstance() {
        static MultiversionPlugin instance;
        return instance;
    }

    ll::plugin::NativePlugin& getSelf() const { return *mSelf; }

    bool load(ll::plugin::NativePlugin& self) {
        mSelf = &self;
        self.getLogger().info("Multiversion bridge loading...");
        return true;
    }

    bool enable() {
        registerPacketHooks(*mSelf);
        registerDisconnectHook();
        mSelf->getLogger().info("Multiversion bridge enabled. Supported range: 1.21.50 - 1.26.10");
        return true;
    }

    bool disable() {
        mSelf->getLogger().info("Multiversion bridge disabled.");
        return true;
    }

private:
    ll::plugin::NativePlugin* mSelf{nullptr};
};

} // namespace multiversion

LL_REGISTER_PLUGIN(multiversion::MultiversionPlugin, multiversion::MultiversionPlugin::getInstance());
