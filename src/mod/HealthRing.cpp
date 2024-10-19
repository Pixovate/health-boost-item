#include "ll/api/Logger.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/actor/monster/AttributeModifier.h"
#include "mc/world/actor/monster/Shulker.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/attribute/Attribute.h"
#include "mc/world/attribute/AttributeInstance.h"
#include "mc/world/attribute/SharedAttributes.h"

ll::Logger logger("HealthRing");

static const mce::UUID    healthStickId   = mce::UUID::fromString("00000000-0000-0000-0000-000000000001");
static std::string const& healthStickName = "HealthStick";

std::unordered_map<int64, int> healthRingCount;

LL_AUTO_TYPE_INSTANCE_HOOK(
    PlayerInit,
    ll::memory::HookPriority::Normal,
    Player,
    "??0Player@@QEAA@AEAVLevel@@AEAVPacketSender@@W4GameType@@_NAEBVNetworkIdentifier@@W4SubClientId@@VUUID@mce@@AEBV?$"
    "basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@7V?$unique_ptr@VCertificate@@U?$default_delete@"
    "VCertificate@@@std@@@9@AEAVEntityContext@@77@Z",
    Player*,
    class Level&                   level,
    class PacketSender&            packetSender,
    ::GameType                     playerGameType,
    bool                           isHostingPlayer,
    class NetworkIdentifier const& owner,
    ::SubClientId                  subid,
    class mce::UUID                uuid,
    std::string const&             playFabId,
    std::string const&             deviceId,
    void*                          certificate,
    class EntityContext&           entityContext,
    std::string const&             platformId,
    std::string const&             platformOnlineId
) {
    auto ori = origin(
        level,
        packetSender,
        playerGameType,
        isHostingPlayer,
        owner,
        subid,
        uuid,
        playFabId,
        deviceId,
        certificate,
        entityContext,
        platformId,
        platformOnlineId
    );

    healthRingCount[ori->getOrCreateUniqueID().id] = 0;
    return ori;
}

LL_AUTO_TYPE_INSTANCE_HOOK(PlayerDe, ll::memory::HookPriority::Normal, Player, "??1Player@@UEAA@XZ", void) {
    auto id = getOrCreateUniqueID();
    healthRingCount.erase(id.id);
}


LL_AUTO_TYPE_INSTANCE_HOOK(PlayerTick, ll::memory::HookPriority::Normal, Player, "?normalTick@Player@@UEAAXXZ", void) {
    auto health = getMutableAttribute(SharedAttributes::HEALTH);
    if (!health) {
        origin();
        return;
    }

    auto& carriedItem  = getCarriedItem();
    auto  itemTypeName = carriedItem.getTypeName();
    auto  itemCount    = carriedItem.mCount;
    auto  playerId     = getOrCreateUniqueID().id;

    if (itemTypeName == "minecraft:stick") {
        auto& previousCount = healthRingCount[playerId];
        if (itemCount != previousCount) {
            previousCount = itemCount;
            AttributeModifier healthStick(
                healthStickId,
                healthStickName,
                2.0f * itemCount,
                AttributeModifierOperation::addition,
                AttributeOperands::Max,
                false
            );
            health->removeModifier(healthStickId);
            health->addModifier(healthStick);
        }
    } else {
        healthRingCount[playerId] = 0;
        health->removeModifier(healthStickId);
    }

    origin();
}
