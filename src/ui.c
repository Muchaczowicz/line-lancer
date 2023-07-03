#include "ui.h"
#include "std.h"
#include "alloc.h"
#include "constants.h"
#include "level.h"
#define CAKE_LAYOUT_IMPLEMENTATION
#include "cake.h"

typedef struct {
    Rectangle area;
    Rectangle warrior;
    Rectangle archer;
    Rectangle support;
    Rectangle special;
    Rectangle resource;
} EmptyDialog;

typedef struct {
    Rectangle area;
    Rectangle label;
    Rectangle upgrade;
    Rectangle demolish;
} BuildingDialog;

void draw_button (
    char      * text,
    Rectangle   area,
    Vector2     cursor,
    float       padding,
    Color       bg,
    Color       hover,
    Color       frame
) {
    if (CheckCollisionPointRec(cursor, area)) {
        DrawRectangleRec(area, hover);
        DrawRectangleLinesEx(area, 2.0f, frame);
    }
    else {
        DrawRectangleRec(area, bg);
        DrawRectangleLinesEx(area, 1.0f, frame);
    }

    DrawText(text, area.x + padding, area.y + padding, UI_FONT_SIZE_BUTTON, frame);
}

float ui_margin () {
    return 5.0f;
}
float ui_spacing () {
    return 2.0f;
}

EmptyDialog empty_dialog (Vector2 position) {
    EmptyDialog result;
    float margin = ui_margin();
    float spacing = ui_spacing();

    Rectangle screen = cake_rect(GetScreenWidth(), GetScreenHeight());
    cake_cut_horizontal(&screen, 0, UI_BAR_SIZE);

    result.area = cake_rect(UI_DIALOG_BUILDING_WIDTH + spacing * 2.0f + margin * 2.0f, UI_DIALOG_BUILDING_HEIGHT + margin * 2.0f + spacing);
    result.area = cake_center_rect(result.area, position.x, position.y);
    cake_clamp_inside(&result.area, screen);

    Rectangle butt = cake_margin_all(result.area, margin);
    Rectangle buttons[6];
    cake_split_grid(butt, spacing, 3, 2, buttons);

    result.warrior  = buttons[0];
    result.archer   = buttons[1];
    result.support  = buttons[2];
    result.special  = buttons[3];
    result.resource = buttons[4];

    return result;
}

BuildingDialog building_dialog (Vector2 position) {
    BuildingDialog result;
    float margin  = ui_margin();
    float spacing = ui_spacing();

    Rectangle screen = cake_rect(GetScreenWidth(), GetScreenHeight());
    cake_cut_horizontal(&screen, 0, UI_BAR_SIZE);

    result.area = cake_rect(UI_DIALOG_UPGRADE_WIDTH + margin * 2.0f, UI_DIALOG_UPGRADE_HEIGHT + margin * 2.0f + spacing * 2.0f);
    result.area = cake_center_rect(result.area, position.x, position.y);
    cake_clamp_inside(&result.area, screen);

    Rectangle butt = cake_margin_all(result.area, margin);
    Rectangle buttons[3];
    cake_split_horizontal(butt, spacing, 3, buttons);

    result.label    = buttons[0];
    result.upgrade  = buttons[1];
    result.demolish = buttons[2];

    return result;
}

Result ui_building_action_click (GameState *const state, Vector2 cursor, BuildingAction * action) {
    BuildingDialog dialog = building_dialog(state->selected_building->position);
    if (CheckCollisionPointRec(cursor, dialog.demolish)) {
        *action = BUILDING_ACTION_DEMOLISH;
    }
    else if (CheckCollisionPointRec(cursor, dialog.upgrade)) {
        *action = BUILDING_ACTION_UPGRADE;
    }
    else {
        *action = BUILDING_ACTION_NONE;
        return FAILURE;
    }
    return SUCCESS;
}

Result ui_building_buy_click (GameState *const state, Vector2 cursor, BuildingType * result) {
    if (state->selected_building->type != BUILDING_EMPTY) {
        return FAILURE;
    }

    Vector2 ui_box = GetWorldToScreen2D(state->selected_building->position, state->camera);
    EmptyDialog dialog = empty_dialog(ui_box);
    if (! CheckCollisionPointRec(cursor, dialog.area)) {
        return FAILURE;
    }

    if (CheckCollisionPointRec(cursor, dialog.warrior)) {
        *result = BUILDING_FIGHTER;
    }
    else if (CheckCollisionPointRec(cursor, dialog.archer)) {
        *result = BUILDING_ARCHER;
    }
    else if (CheckCollisionPointRec(cursor, dialog.support)) {
        *result = BUILDING_SUPPORT;
    }
    else if (CheckCollisionPointRec(cursor, dialog.special)) {
        *result = BUILDING_SPECIAL;
    }
    else if (CheckCollisionPointRec(cursor, dialog.resource)) {
        *result = BUILDING_RESOURCE;
    }
    else {
        return FAILURE;
    }

    return SUCCESS;
}

void render_empty_building_dialog (GameState *const state) {
    Vector2 cursor = GetMousePosition();
    Vector2 ui_box = GetWorldToScreen2D(state->selected_building->position, state->camera);
    EmptyDialog dialog = empty_dialog(ui_box);

    {
        Color dialog_bg;

        if (CheckCollisionPointRec(cursor, dialog.area)) {
            dialog_bg = BLUE;
        } else {
            dialog_bg = DARKBLUE;
        }

        DrawRectangleRec(dialog.area, dialog_bg);
    }

    {
        float margin = ui_margin();
        Color color_bg = DARKBLUE;
        Color color_hover = LIGHTGRAY;
        Color color_frame = BLACK;

        draw_button("Fighter", dialog.warrior, cursor, margin, color_bg, color_hover, color_frame);
        draw_button("Archer", dialog.archer, cursor, margin, color_bg, color_hover, color_frame);
        draw_button("Support", dialog.support, cursor, margin, color_bg, color_hover, color_frame);

        draw_button("Special", dialog.special, cursor, margin, color_bg, color_hover, color_frame);
        draw_button("Cash", dialog.resource, cursor, margin, color_bg, color_hover, color_frame);
    }
}

void render_upgrade_building_dialog (GameState *const state) {
    Vector2 cursor = GetMousePosition();
    BuildingDialog dialog = building_dialog(state->selected_building->position);

    Color dialog_bg;
    if (CheckCollisionPointRec(cursor, dialog.area)) {
        dialog_bg = BLUE;
    } else {
        dialog_bg = DARKBLUE;
    }

    DrawRectangleRec(dialog.area, dialog_bg);
    Rectangle level = cake_cut_horizontal(&dialog.label, 2.0f, 0.6f);
    char * text;
    switch (state->selected_building->type) {
        case BUILDING_FIGHTER: {
            text = "Fighter";
        } break;
        case BUILDING_ARCHER: {
            text = "Archer";
        } break;
        case BUILDING_SUPPORT: {
            text = "Support";
        } break;
        case BUILDING_SPECIAL: {
            text = "Special";
        } break;
        case BUILDING_RESOURCE: {
            text = "Cash";
        } break;
        case BUILDING_EMPTY: {
            DrawText("Invalid", dialog.label.x, dialog.label.y, 20, WHITE);
        } return;
    }
    char * lvl;
    switch (state->selected_building->upgrades) {
        case 0: {
            lvl = "Level 1";
        } break;
        case 1: {
            lvl = "Level 2";
        } break;
        case 2: {
            lvl = "Level 3";
        } break;
        default: {
            lvl = "Level N/A";
        } break;
    }
    DrawText(text, dialog.label.x, dialog.label.y, 20, WHITE);
    DrawText(lvl, level.x, level.y, 16, WHITE);

    float margin = ui_margin();
    Color color_bg = DARKBLUE;
    Color color_hover = LIGHTGRAY;
    Color color_frame = BLACK;
    draw_button("Upgrade", dialog.upgrade, cursor, margin, color_bg, color_hover, color_frame);
    draw_button("Demolish", dialog.demolish, cursor, margin, color_bg, color_hover, color_frame);
}

void render_resource_bar (GameState *const state) {
    usize player_index;
    if (get_local_player_index(state, &player_index)) {
        player_index = 1;
    }
    PlayerData * player = &state->players.items[player_index];

    int income = get_expected_income(state->current_map, player_index);

    Rectangle bar = cake_rect(GetScreenWidth(), UI_BAR_SIZE);
    Rectangle mbar = cake_margin_all(bar, UI_BAR_MARGIN);

    char * gold_label   = "Gold: ";
    char * gold         = convert_int_to_ascii(player->resource_gold, &temp_alloc);
    char * income_label = convert_int_to_ascii(income, &temp_alloc);

    int gold_label_width = MeasureText(gold_label, UI_FONT_SIZE_BAR);
    int gold_width       = MeasureText(gold, UI_FONT_SIZE_BAR);

    Rectangle label_rect = cake_cut_vertical(&mbar, 0, gold_label_width);
    Rectangle gold_rect  = cake_cut_vertical(&mbar, 0, gold_width);

    DrawRectangleRec(bar, DARKGRAY);
    DrawText(gold_label   , label_rect.x, label_rect.y, UI_FONT_SIZE_BAR, WHITE);
    DrawText(gold         , gold_rect.x , gold_rect.y , UI_FONT_SIZE_BAR, WHITE);
    cake_cut_vertical(&mbar, 0, UI_BAR_MARGIN);

    if (income >= 0) {
        int sign_width = MeasureText("+", UI_FONT_SIZE_BAR);
        Rectangle sign = cake_cut_vertical(&mbar, 0, sign_width);
        DrawText("+", sign.x, sign.y, UI_FONT_SIZE_BAR, WHITE);
    }

    DrawText(income_label, mbar.x, mbar.y, UI_FONT_SIZE_BAR, WHITE);
}

void render_ui (GameState *const state) {
    if (state->current_input == INPUT_OPEN_BUILDING) {
        if (state->selected_building->type == BUILDING_EMPTY) {
            render_empty_building_dialog(state);
        }
        else {
            render_upgrade_building_dialog(state);
        }
    }
    render_resource_bar(state);
}
