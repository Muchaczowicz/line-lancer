#include "game.h"
#include "units.h"
#include "level.h"
#include "constants.h"
#include <raymath.h>


void spawn_unit(GameState * state, Building * building) {
    Unit * unit = unit_from_building(building);
    if (unit == NULL) {
        return;
    }
    listUnitAppend(&state->units, unit);
    TraceLog(LOG_INFO, "Unit spawned for player %zu", unit->player_owned);
}

void spawn_units(GameState * state, float delta_time) {
    for (usize r = 0; r < state->current_map->regions.len; r++) {
        ListBuilding * buildings = &state->current_map->regions.items[r].buildings;

        for (usize b = 0; b < buildings->len; b++) {
            Building * building = &buildings->items[b];
            if (building->type == BUILDING_EMPTY)
                continue;

            building->spawn_timer += delta_time;

            if (building->spawn_timer >= 6.0f) {
                building->spawn_timer = 0.0f;
                spawn_unit(state, building);
            }
        }
    }
}

usize find_unit(ListUnit * units, Unit * unit) {
    for (usize a = 0; a < units->len; a++) {
        if (units->items[a] == unit) {
            return a;
        }
    }
    return units->len;
}

void update_unit_state(GameState * state) {
    for (usize u = 0; u < state->units.len; u++) {
        Unit * unit = state->units.items[u];

        switch (unit->state) {
            case UNIT_STATE_MOVING: {
                float distance = Vector2Distance(unit->position, unit->location->position);
                if (distance < 0.1f) {
                    if (get_enemy_in_range(unit)) {
                        unit->state = UNIT_STATE_FIGHTING;
                    }
                    else {
                        move_unit_forward(unit);
                    }
                }
            } break;
            case UNIT_STATE_GUARDING: {
                // do nothing, guardians are always in this state, regular units never enter this state
            } break;
            case UNIT_STATE_FIGHTING: {
                if (get_enemy_in_range(unit) == NULL) {
                    unit->state = UNIT_STATE_MOVING;
                }
            } break;
        }
    }
}

void move_units(GameState * state, float delta_time) {
    for (usize u = 0; u < state->units.len; u++) {
        Unit * unit = state->units.items[u];

        switch (unit->state) {
            case UNIT_STATE_MOVING: {
                unit->position = Vector2MoveTowards(unit->position, unit->location->position, UNIT_SPEED * delta_time);
            } break;
            default: break;
        }
    }
}

void units_fight(GameState * state, float delta_time) {
    for (usize i = 0; i < state->units.len; i++) {
        Unit * unit = state->units.items[i];
        if (unit->state != UNIT_STATE_FIGHTING)
            continue;
        Unit * target = get_enemy_in_range(unit);
        if (target) {
            target->health -= get_unit_attack(unit) * delta_time;
            if (target->health > 0.0f) {
                continue;
            }
            if (target->state == UNIT_STATE_GUARDING) {
                Region * region = region_by_guardian(&state->current_map->regions, target);
                region_change_ownership(region, unit->player_owned);
            }
            else {
                usize target_index = destroy_unit(&state->units, target);
                if (target_index < i) i--;
            }
        }
    }
}

void simulate_units(GameState * state) {
    float dt = GetFrameTime();

    spawn_units       (state, dt);
    move_units        (state, dt);
    units_fight       (state, dt);
    // TODO handle guardians fighting
    update_unit_state (state);
}
