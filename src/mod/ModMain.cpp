#include "mod/ModMain.h"

#include <memory>

#include "ll/api/mod/RegisterHelper.h"

namespace health_boost_item {

static std::unique_ptr<HealthBoostItem> instance;

HealthBoostItem& HealthBoostItem::getInstance() { return *instance; }

bool HealthBoostItem::load() {
    getSelf().getLogger().debug("Loading...");
    // Code for loading the mod goes here.
    return true;
}

bool HealthBoostItem::enable() {
    getSelf().getLogger().debug("Enabling...");
    // Code for enabling the mod goes here.
    return true;
}

bool HealthBoostItem::disable() {
    getSelf().getLogger().debug("Disabling...");
    // Code for disabling the mod goes here.
    return true;
}

} // namespace health_boost_item

LL_REGISTER_MOD(health_boost_item::HealthBoostItem, health_boost_item::instance);
