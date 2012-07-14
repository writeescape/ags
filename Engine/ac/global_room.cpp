
#include "ac/global_room.h"
#include "wgt2allg.h"
#include "ac/ac_common.h"
#include "ac/character.h"
#include "ac/characterinfo.h"
#include "ac/draw.h"
#include "ac/event.h"
#include "ac/gamesetupstruct.h"
#include "ac/gamestate.h"
#include "ac/global_character.h"
#include "ac/global_game.h"
#include "ac/movelist.h"
#include "ac/properties.h"
#include "ac/room.h"
#include "ac/roomstatus.h"
#include "debug/debug.h"
#include "script/script.h"

extern GameState play;
extern GameSetupStruct game;
extern CharacterInfo*playerchar;
extern int displayed_room;
extern int in_enters_screen;
extern int in_leaves_screen;
extern int in_inv_screen, inv_screen_newroom;
extern MoveList *mls;
extern int gs_to_newroom;
extern RoomStatus *roomstats;
extern roomstruct thisroom;

void SetAmbientTint (int red, int green, int blue, int opacity, int luminance) {
    if ((red < 0) || (green < 0) || (blue < 0) ||
        (red > 255) || (green > 255) || (blue > 255) ||
        (opacity < 0) || (opacity > 100) ||
        (luminance < 0) || (luminance > 100))
        quit("!SetTint: invalid parameter. R,G,B must be 0-255, opacity & luminance 0-100");

    DEBUG_CONSOLE("Set ambient tint RGB(%d,%d,%d) %d%%", red, green, blue, opacity);

    play.rtint_red = red;
    play.rtint_green = green;
    play.rtint_blue = blue;
    play.rtint_level = opacity;
    play.rtint_light = (luminance * 25) / 10;
}

void NewRoom(int nrnum) {
    if (nrnum < 0)
        quitprintf("!NewRoom: room change requested to invalid room number %d.", nrnum);

    if (displayed_room < 0) {
        // called from game_start; change the room where the game will start
        playerchar->room = nrnum;
        return;
    }


    DEBUG_CONSOLE("Room change requested to room %d", nrnum);
    EndSkippingUntilCharStops();

    can_run_delayed_command();

    if (play.stop_dialog_at_end != DIALOG_NONE) {
        if (play.stop_dialog_at_end == DIALOG_RUNNING)
            play.stop_dialog_at_end = DIALOG_NEWROOM + nrnum;
        else
            quit("!NewRoom: two NewRoom/RunDialog/StopDialog requests within dialog");
        return;
    }

    if (in_leaves_screen >= 0) {
        // NewRoom called from the Player Leaves Screen event -- just
        // change which room it will go to
        in_leaves_screen = nrnum;
    }
    else if (in_enters_screen) {
        setevent(EV_NEWROOM,nrnum);
        return;
    }
    else if (in_inv_screen) {
        inv_screen_newroom = nrnum;
        return;
    }
    else if ((inside_script==0) & (in_graph_script==0)) {
        new_room(nrnum,playerchar);
        return;
    }
    else if (inside_script) {
        curscript->queue_action(ePSANewRoom, nrnum, "NewRoom");
        // we might be within a MoveCharacterBlocking -- the room
        // change should abort it
        if ((playerchar->walking > 0) && (playerchar->walking < TURNING_AROUND)) {
            // nasty hack - make sure it doesn't move the character
            // to a walkable area
            mls[playerchar->walking].direct = 1;
            StopMoving(game.playercharacter);
        }
    }
    else if (in_graph_script)
        gs_to_newroom = nrnum;
}


void NewRoomEx(int nrnum,int newx,int newy) {

    Character_ChangeRoom(playerchar, nrnum, newx, newy);

}

void NewRoomNPC(int charid, int nrnum, int newx, int newy) {
    if (!is_valid_character(charid))
        quit("!NewRoomNPC: invalid character");
    if (charid == game.playercharacter)
        quit("!NewRoomNPC: use NewRoomEx with the player character");

    Character_ChangeRoom(&game.chars[charid], nrnum, newx, newy);
}

void ResetRoom(int nrnum) {
    if (nrnum == displayed_room)
        quit("!ResetRoom: cannot reset current room");
    if ((nrnum<0) | (nrnum>=MAX_ROOMS))
        quit("!ResetRoom: invalid room number");
    if (roomstats[nrnum].beenhere) {
        if (roomstats[nrnum].tsdata!=NULL)
            free(roomstats[nrnum].tsdata);
        roomstats[nrnum].tsdata=NULL;
        roomstats[nrnum].tsdatasize=0;
    }
    roomstats[nrnum].beenhere=0;
    DEBUG_CONSOLE("Room %d reset to original state", nrnum);
}

int HasPlayerBeenInRoom(int roomnum) {
    if ((roomnum < 0) || (roomnum >= MAX_ROOMS))
        return 0;
    return roomstats[roomnum].beenhere;
}

void CallRoomScript (int value) {
    can_run_delayed_command();

    if (!inside_script)
        quit("!CallRoomScript: not inside a script???");

    play.roomscript_finished = 0;
    curscript->run_another("$on_call", value, 0);
}

int HasBeenToRoom (int roomnum) {
    if ((roomnum < 0) || (roomnum >= MAX_ROOMS))
        quit("!HasBeenToRoom: invalid room number specified");

    if (roomstats[roomnum].beenhere)
        return 1;
    return 0;
}

int GetRoomProperty (const char *property) {
    return get_int_property (&thisroom.roomProps, property);
}

void GetRoomPropertyText (const char *property, char *bufer) {
    get_text_property (&thisroom.roomProps, property, bufer);
}

void SetBackgroundFrame(int frnum) {
    if ((frnum<-1) | (frnum>=thisroom.num_bscenes))
        quit("!SetBackgrondFrame: invalid frame number specified");
    if (frnum<0) {
        play.bg_frame_locked=0;
        return;
    }

    play.bg_frame_locked = 1;

    if (frnum == play.bg_frame)
    {
        // already on this frame, do nothing
        return;
    }

    play.bg_frame = frnum;
    on_background_frame_change ();
}

int GetBackgroundFrame() {
    return play.bg_frame;
}