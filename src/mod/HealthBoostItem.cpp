#include "ll/api/Logger.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/actor/monster/AttributeModifier.h"
#include "mc/world/actor/monster/Shulker.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/attribute/AttributeInstance.h"
#include "mc/world/attribute/SharedAttributes.h"

ll::Logger logger("HealthBoostItem");

static const mce::UUID    healthAppleId   = mce::UUID::fromString("00000000-0000-0000-0000-000000000001");
static std::string const& healthAppleName = "HealthApple";

static const mce::UUID    healthStickId   = mce::UUID::fromString("00000000-0000-0000-0000-000000000002");
static std::string const& healthStickName = "HealthStick";

std::unordered_map<int64, int> healthAppleCount;
std::unordered_map<int64, int> healthStickCount;

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

    healthAppleCount[ori->getOrCreateUniqueID().id] = 0;
    healthStickCount[ori->getOrCreateUniqueID().id] = 0;
    return ori;
}

LL_AUTO_TYPE_INSTANCE_HOOK(PlayerDe, ll::memory::HookPriority::Normal, Player, "??1Player@@UEAA@XZ", void) {
    auto id = getOrCreateUniqueID();
    healthAppleCount.erase(id.id);
    healthStickCount.erase(id.id);
}

LL_AUTO_TYPE_INSTANCE_HOOK(PlayerTick, ll::memory::HookPriority::Normal, Player, "?normalTick@Player@@UEAAXXZ", void) {
    auto health = getMutableAttribute(SharedAttributes::HEALTH);
    if (!health) {
        origin();
        return;
    }

    auto playerId   = getOrCreateUniqueID().id;
    auto stickCount = 0;
    auto appleCount = 0;

    for (const auto& it : getInventory()) {
        if (it.getTypeName() == "minecraft:stick") {
            stickCount += it.mCount;
        } else if (it.getTypeName() == "minecraft:apple") {
            appleCount += it.mCount;
        }
    }

    auto updateHealthModifier =
        [&](auto& count, auto& countMap, const auto& modifierId, const std::string& modifierName, float multiplier) {
            if (health->hasModifier(modifierId) && count != countMap[playerId]) {
                countMap[playerId] = count;
                AttributeModifier modifier(
                    modifierId,
                    modifierName,
                    multiplier * count,
                    AttributeModifierOperation::addition,
                    AttributeOperands::Max,
                    false
                );
                health->updateModifier(modifier);
            } else if (!health->hasModifier(modifierId) && count > 0) {
                countMap[playerId] = count;
                AttributeModifier modifier(
                    modifierId,
                    modifierName,
                    multiplier * count,
                    AttributeModifierOperation::addition,
                    AttributeOperands::Max,
                    false
                );
                health->addModifier(modifier);
            } else if (health->hasModifier(modifierId) && count == 0) {
                countMap[playerId] = count;
                health->removeModifier(modifierId);
            }
        };

    updateHealthModifier(stickCount, healthStickCount, healthStickId, healthStickName, 2.0f);
    updateHealthModifier(appleCount, healthAppleCount, healthAppleId, healthAppleName, 4.0f);

    origin();
}
