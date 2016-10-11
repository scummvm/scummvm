/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TITANIC_GAME_OBJECT_H
#define TITANIC_GAME_OBJECT_H

#include "audio/mixer.h"
#include "common/stream.h"
#include "titanic/core/named_item.h"
#include "titanic/sound/proximity.h"
#include "titanic/support/mouse_cursor.h"
#include "titanic/support/credit_text.h"
#include "titanic/support/movie_range_info.h"
#include "titanic/support/rect.h"
#include "titanic/support/strings.h"
#include "titanic/support/movie_clip.h"
#include "titanic/pet_control/pet_section.h"
#include "titanic/pet_control/pet_text.h"
#include "titanic/game_state.h"

namespace Titanic {

enum Find { FIND_GLOBAL = 1, FIND_ROOM = 2, FIND_PET = 4, FIND_MAILMAN = 8 };
enum Found { FOUND_NONE = 0, FOUND_GLOBAL = 1, FOUND_ROOM = 2, FOUND_PET = 3, FOUND_MAILMAN = 4 };

class CDontSaveFileItem;
class CMailMan;
class CMusicRoom;
class CRoomItem;
class CStarControl;
class CMouseDragStartMsg;
class CTrueTalkNPC;
class CVideoSurface;
class OSMovie;

class CGameObject : public CNamedItem {
	friend class OSMovie;
	DECLARE_MESSAGE_MAP;
private:
	static int _soundHandles[4];
private:
	/**
	 * Load a visual resource for the object
	 */
	void loadResource(const CString &name);

	/**
	 * Process and remove any registered movie range info
	 */
	void processMoveRangeInfo();

	/**
	 * Merges one rect into another, and returns true if there was
	 * a common intersection
	 */
	bool clipRect(const Rect &rect1, Rect &rect2) const;
protected:
	static CCreditText *_credits;
protected:
	double _field34;
	double _field38;
	double _field3C;
	int _field40;
	int _field44;
	int _field48;
	int _field4C;
	CMovieClipList _movieClips;
	int _initialFrame;
	CMovieRangeInfoList _movieRangeInfoList;
	int _frameNumber;
	CPetText *_text;
	uint _textBorder;
	uint _textBorderRight;
	int _field9C;
	Common::Point _savedPos;
	CVideoSurface *_surface;
	CString _resource;
	int _fieldB8;
protected:
	/**
	 * Saves the current position the object is located at
	 */
	void savePosition();

	/**
	 * Resets the object back to the previously saved starting position
	 */
	void resetPosition();

	/**
	 * Check for starting to drag the object
	 */
	bool checkStartDragging(CMouseDragStartMsg *msg);

	/**
	 * Goto a new view
	 */
	void gotoView(const CString &viewName, const CString &clipName);

	/**
	 * Parses a view into it's components of room, node, and view,
	 * and locates the designated view
	 */
	CViewItem * parseView(const CString &viewString);

	/**
	 * Loads a movie
	 */
	void loadMovie(const CString &name, bool pendingFlag = true);

	/**
	 * Loads an image
	 */
	void loadImage(const CString &name, bool pendingFlag = true);

	void inc54();
	void dec54();

	/**
	 * Locks/hides the mouse
	 */
	void lockMouse();

	/**
	 * Unlocks/shows the mouse
	 */
	void unlockMouse();

	/**
	 * Hides the mouse cursor
	 */
	void hideMouse();

	/**
	 * Shows the mouse cursor
	 */
	void showMouse();

	/**
	 * Disable the mouse
	 */
	void disableMouse();

	/**
	 * Enable the mouse
	 */
	void enableMouse();

	void mouseLockE4();
	void mouseUnlockE4();

	/**
	 * Sets the mouse to a new position
	 */
	void mouseSetPosition(const Point &pt, double rate);

	/**
	 * Lock the input handler
	 */
	void lockInputHandler();

	/**
	 * Unlock the input handler
	 */
	void unlockInputHandler();

	/**
	 * Load a sound
	 */
	void loadSound(const CString &name);

	/**
	 * Plays a sound
	 * @param name		Filename of sound to play
	 * @param volume	Volume level
	 * @param balance	Sound balance (not actually used in original)
	 * @param repeated	If true, sound will repeat indefinitely
	 */
	int playSound(const CString &name, uint volume = 100, int balance = 0, bool repeated = false);

	/**
	 * Plays a sound
	 * @param name		Filename of sound to play
	 * @param prox		Proximity object with the sound data
	 */
	int playSound(const CString &name, CProximity &prox);

	/**
	 * Queues a sound to play after a specified one finishes
	 * @param name			Filename of sound to play
	 * @param priorHandle	Sound to wait until finished before playing
	 * @param volume		Volume level
	 * @param balance		Sound balance (not actually used by original)
	 * @param repeated		If true, sound will repeat indefinitely
	 */
	int queueSound(const CString &name, uint priorHandle, uint volume = 100,
		int balance = 0, bool repeated = false);

	/**
	 * Stop a sound
	 * @param handle	Sound handle
	 * @param seconds	Optional number of seconds to transition sound off
	 */
	void stopSound(int handle, uint seconds = 0);

	/**
	 * Returns true if a sound with the specified handle is active
	 */
	bool isSoundActive(int handle) const;

	/**
	 * Sets the volume for a sound
	 * @param handle	Sound handle
	 * @param volume	Volume percentage (0 to 100)
	 * @param seconds	Number of seconds to transition to the new volume
	 */
	void setSoundVolume(int handle, uint percent, uint seconds);

	/**
	 * Plays a sound, and saves it's handle in the global sound handles list
	 * @param resName		Filename of sound to play
	 * @param mode			Volume mode level
	 * @param initialMute	If set, sound transitions in from mute over 2 seconds
	 * @param repeated		Flag for repeating sounds
	 * @param handleIndex	Slot 0 to 3 in the shared sound handle list to store the sound's handle
	 * @param soundType		Specifies whether the sound is a sound effect or music
	 */
	void playGlobalSound(const CString &resName, int mode, bool initialMute, bool repeated,
		int handleIndex, Audio::Mixer::SoundType soundType = Audio::Mixer::kMusicSoundType);

	/**
	 * Stops a sound saved in the global sound handle list
	 * @param transition	If set, the sound transitions to silent before stopping
	 * @param handleIndex	Index of sound to stop. If -1, all global sounds are stopped
	 */
	void stopGlobalSound(bool transition, int handleIndex);

	/**
	 * Updates the volume for a global sound based on the specified mode's volume
	 * @param mode			Volume level mode
	 * @param seconds		Number of seconds to transition to new volume
	 * @param handleIndex	Index of global sound to update. If -1, all global sounds are updated
	 */
	void setGlobalSoundVolume(int mode, uint seconds, int handleIndex);

	void sound8(bool flag) const;

	/**
	 * Adds a timer
	 */
	int addTimer(int endVal, uint firstDuration, uint repeatDuration);

	/**
	 * Adds a timer
	 */
	int addTimer(uint firstDuration, uint repeatDuration = 0);

	/**
	 * Stops a timer
	 */
	void stopTimer(int id);

	/**
	 * Start an animation timer
	 */
	int startAnimTimer(const CString &action, uint firstDuration, uint repeatDuration = 0);

	/**
	 * Stop an animation timer
	 */
	void stopAnimTimer(int id);

	/**
	 * Causes the game to sleep for the specified time
	 */
	void sleep(uint milli);

	/**
	 * Get the current mouse cursor position
	 */
	Point getMousePos() const;

	/*
	 * Compares the current view's name in a Room.Node.View tuplet
	 * string form to the passed string
	 */
	bool compareViewNameTo(const CString &name) const;

	/**
	 * Compare the name of the parent room to the item to a passed string
	 */
	int compareRoomNameTo(const CString &name);

	/**
	 * Gets the first object under the system MailMan
	 */
	CGameObject *getMailManFirstObject() const;

	/**
	 * Gets the next object under the system MailMan
	 */
	CGameObject *getMailManNextObject(CGameObject *prior) const;

	/**
	 * Find mail by room flags
	 */
	CGameObject *findMailByFlags(int mode, uint roomFlags);

	/**
	 * Find next mail from a given prior one
	 */
	CGameObject *getNextMail(CGameObject *prior);

	/**
	 * Finds an object by name within the object's room
	 */
	CGameObject *findRoomObject(const CString &name) const;

	/**
	 * FInds an object under the current room
	 */
	CGameObject *findInRoom(const CString &name);

	/**
	 * Moves the object to be under the current view
	 */
	void moveToView();

	/**
	 * Moves the object to be under the specified view
	 */
	void moveToView(const CString &name);

	/**
	 * Change the view
	 */
	bool changeView(const CString &viewName, const CString &clipName);

	/**
	 * Change the view
	 */
	bool changeView(const CString &viewName);

	/**
	 * Play an arbitrary clip
	 */
	void playClip(const CString &name, uint flags = 0);

	/**
	 * Play a clip
	 */
	void playClip(uint startFrame, uint endFrame);

	/**
	 * Play a cutscene
	 */
	void playCutscene(uint startFrame, uint endFrame);

	/**
	 * Play a clip randomly from a passed list of names
	 */
	void playRandomClip(const char *const *names, uint flags = 0);

	/**
	 * Return the current view/node/room as a single string
	 */
	CString getViewFullName() const;

	/**
	 * Returns true if a clip exists in the list with a given name
	 * and starting frame number
	 */
	bool clipExistsByStart(const CString &name, int startFrame = 0) const;

	/**
	 * Returns true if a clip exists in the list with a given name
	 * and ending frame number
	 */
	bool clipExistsByEnd(const CString &name, int endFrame = 0) const;

	/**
	 * Clear the PET display
	 */
	void petClear() const;

	/**
	 * Returns the MailMan
	 */
	CMailMan *getMailMan() const;

	/**
	 * Gets the don't save container object
	 */
	CDontSaveFileItem *getDontSave() const;

	/**
	 * Returns a child of the Dont Save area of the project of the given class
	 */
	CTreeItem *getDontSaveChild(ClassDef *classDef) const;

	/**
	 * Returns the special hidden room container
	 */
	CRoomItem *getHiddenRoom() const;

	/**
	 * Locates a room with the given name
	 */
	CRoomItem *locateRoom(const CString &name) const;

	/**
	 * Scan the specified room for an item by name
	 */
	CTreeItem *findUnder(CTreeItem *parent, const CString &name) const;

	/**
	 * Finds a room by name
	 */
	CRoomItem *findRoomByName(const CString &name);

	/**
	 * Returns the music room instance from the game manager
	 */
	CMusicRoom *getMusicRoom() const;

	/**
	 * Set's the player's passenger class
	 */
	void setPassengerClass(int newClass);

	/**
	 * Overrides whether the object's movie has audio timing
	 */
	void movieSetAudioTiming(bool flag);

	void fn10(int v1, int v2, int v3);

	/**
	 * Gets the duration of a specified clip in milliseconds
	 */
	int getClipDuration(const CString &name, int frameRate = 14) const;

	/**
	 * Returns the current system tick count
	 */
	uint32 getTicksCount();

	/**
	 * Gets a resource from the DAT file
	 */
	Common::SeekableReadStream *getResource(const CString &name);

	/**
	 * Returns true if a mail with a specified Id exists
	 */
	bool mailExists(int id) const;

	/**
	 * Returns a specified mail, if one exists
	 */
	CGameObject *findMail(int id) const;

	/**
	 * Resets the Mail Man value
	 */
	void resetMail();

	/**
	 * Locks the PET, disabling all input. Can be called multiple times
	 */
	void petLockInput();

	/**
	 * Unlocks PET input
	 */
	void petUnlockInput();

	/**
	 * Flag to quit the game
	 */
	void quitGame();

	/**
	 * Set the frame rate for the currently loaded movie
	 */
	void setMovieFrameRate(double rate);

	/**
	 * Set up the text and borders for the object
	 */
	void setText(const CString &str, int border = 0, int borderRight = 0);

	/**
	 * Sets whether the text will use borders
	 */
	void setTextHasBorders(bool hasBorders);

	/**
	 * Sets the bounds for a previously defined text area
	 */
	void setTextBounds();

	/**
	 * Sets the color for the object's text
	 */
	void setTextColor(byte r, byte g, byte b);

	/**
	 * Sets the font number to use for text
	 */
	void setTextFontNumber(int fontNumber);

	/**
	 * Gets the width of the text contents
	 */
	int getTextWidth() const;

	/**
	 * Returns the text cursor
	 */
	CTextCursor *getTextCursor() const;

	/**
	 * Scroll text up
	 */
	void scrollTextUp();

	/**
	 * Scroll text down
	 */
	void scrollTextDown();

	/**
	 * Gets a new random number
	 */
	int getRandomNumber(int max, int *oldVal = nullptr);
public:
	Rect _bounds;
	bool _isMail;
	int _id;
	uint _roomFlags;
	int _field60;
	CursorId _cursorId;
	bool _visible;
public:
	/**
	 * Initializes statics
	 */
	static void init();

	/**
	 * Deinitializes statics
	 */
	static void deinit();
public:
	CLASSDEF;
	CGameObject();
	~CGameObject();

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent);

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);

	/**
	 * Returns the clip list, if any, associated with the item
	 */
	virtual const CMovieClipList *getMovieClips() const { return &_movieClips; }

	/**
	 * Allows the item to draw itself
	 */
	virtual void draw(CScreenManager *screenManager);

	/**
	 * Gets the bounds occupied by the item
	 */
	virtual Rect getBounds() const;

	/**
	 * Called when the view changes
	 */
	virtual void viewChange();

	/**
	 * Allows the item to draw itself
	 */
	void draw(CScreenManager *screenManager, const Rect &destRect, const Rect &srcRect);

	/**
	 * Allows the item to draw itself
	 */
	void draw(CScreenManager *screenManager, const Point &destPos);

	/**
	 * Allows the item to draw itself
	 */
	void draw(CScreenManager *screenManager, const Point &destPos, const Rect &srcRect);

	/**
	 * Returns true if the item is the PET control
	 */
	virtual bool isPet() const;

	/**
	 * Checks the passed point is validly in the object,
	 * with extra checking of object flags status
	 */
	bool checkPoint(const Point &pt, bool ignore40 = false, bool visibleOnly = false);

	/**
	 * Set the position of the object
	 */
	void setPosition(const Point &newPos);

	/**
	 * Get the centre of the game object's bounds
	 */
	Point getControid() const;

	/**
	 * Change the object's status
	 */
	void playMovie(uint flags);

	/**
	 * Play the movie specified in _resource
	 */
	void playMovie(int startFrame, int endFrame, uint flags);

	/**
	 * Play the movie specified in _resource
	 */
	void playMovie(int startFrame, int endFrame, int initialFrame, uint flags);

	/**
	 * Returns true if the object has a currently active movie
	 */
	bool hasActiveMovie() const;

	/**
	 * Stops any movie currently playing for the object
	 */
	void stopMovie();

	/**
	 * Get the current movie frame
	 */
	int getMovieFrame() const;

	/**
	 * Returns the object's frame number
	 */
	int getFrameNumber() const { return _frameNumber; }

	/**
	 * Loads a frame
	 */
	void loadFrame(int frameNumber);

	/**
	 * Load the surface
	 */
	void loadSurface();

	/**
	 * Marks the area occupied by the object as dirty, requiring re-rendering
	 */
	void makeDirty();

	/**
	 * Marks the area in the passed rect as dirty, and requiring re-rendering
	 */
	void makeDirty(const Rect &r);

	/**
	 * Sets whether the object is visible
	 */
	void setVisible(bool val);

	/**
	 * Return the player's passenger class
	 */
	int getPassengerClass() const;

	/**
	 * Return the player's previous passenger class
	 */
	int getPriorClass() const;

	/**
	 * Sets the mail identifier for an object
	 */
	void setMailId(int mailId);

	/**
	 * Returns true if there's an attached surface which has a frame
	 * ready for display
	 */
	bool surfaceHasFrame() const;

	/**
	 * Finds an item in various system areas
	 */
	Found find(const CString &name, CGameObject **item, int findAreas);

	/**
	 * Returns a hidden object
	 */
	CGameObject *getHiddenObject(const CString &name) const;

	/**
	 * Sets up credits text
	 */
	void createCredits();

	/**
	 * Support function for drag moving
	 */
	void dragMove(const Point &pt);

	/**
	 * Returns the currently dragging item (if any) if it's a game object
	 */
	CGameObject *getDraggingObject() const;

	bool compareRoomFlags(int mode, uint flags1, uint flags2);

	/*--- CGameManager Methods ---*/

	/**
	 * Return the current room
	 */
	CRoomItem *getRoom() const;

	/**
	 * Return the current node
	 */
	CNodeItem *getNode() const;

	/**
	 * Return the current room
	 */
	CViewItem *getView() const;

	/**
	 * Get the current room name
	 */
	CString getRoomName() const;

	/**
	 * Get the current node and room in the form "room.node"
	 */
	CString getRoomNodeName() const;

	/**
	 * Adds an object to the mail list
	 */
	void addMail(int mailId);

	/**
	 * Remove an object from the mail list
	 */
	void removeMail(int id, int v);

	/**
	 * Return the full Id of the current view in a
	 * room.node.view tuplet form
	 */
	CString getFullViewName();

	/*--- CPetControl Methods ---*/

	/**
	 * Returns the PET control
	 */
	CPetControl *getPetControl() const;

	/**
	 * Moves a specified item to the carry parcel
	 */
	void petAddToCarryParcel(CGameObject *obj);

	/**
	 * Add the item to the inventory
	 */
	void petAddToInventory();

	CTreeItem *petContainerRemove(CGameObject *obj);

	bool petCheckNode(const CString &name);

	/**
	 * Dismiss a bot
	 */
	bool petDismissBot(const CString &name);

	/**
	 * Is Doorbot or Bellbot present in the current view
	 */
	bool petDoorOrBellbotPresent() const;

	/**
	 * Display a message in the PET
	 */
	void petDisplayMessage(int unused, StringId stringId);

	/**
	 * Display a message in the PET
	 */
	void petDisplayMessage(int unused, const CString &str);

	/**
	 * Display a message in the PET
	 */
	void petDisplayMessage(StringId stringId, int param = 0);

	/**
	 * Display a message in the PET
	 */
	void petDisplayMessage(const CString &str, int param = 0);

	/**
	 * Gets the entry number used when last arriving at the well
	 */
	int petGetRoomsWellEntry() const;

	/**
	 * Hide the PET
	 */
	void petHide();

	/**
	 * Hides the text cursor in the current section, if applicable
	 */
	void petHideCursor();

	/**
	 * Highlights a glyph in the currently active PET section
	 */
	void petHighlightGlyph(int id);

	/**
	 * Called when the status of an item in the inventory has changed
	 */
	void petInvChange();

	/**
	 * Moves the item from it's original position to be under the hidden room
	 */
	void petMoveToHiddenRoom();

	/**
	 * Gives the player a new assigned room in the specified passenger class
	 */
	void petReassignRoom(int passClassNum);

	/**
	 * Sets a new area in the PET
	 */
	void petSetArea(PetArea newArea) const;

	/**
	 * Set the remote target in the PET to this object
	 */
	void petSetRemoteTarget();

	/**
	 * Sets the entry number for arriving at the well
	 */
	void petSetRoomsWellEntry(int entryNum);

	void petSetRooms1D4(int v);


	/**
	 * Show the PET
	 */
	void petShow();

	/**
	 * Shows the text cursor in the current section, if applicable
	 */
	void petShowCursor();

	/**
	 * Summon a bot
	 */
	void petOnSummonBot(const CString &name, int val);

	/*--- CStarControl Methods ---*/

	/**
	 * Returns the star control
	 */
	CStarControl *getStarControl() const;

	void starFn1(int v);
	bool starFn2();

	/*--- CTrueTalkManager Methods ---*/

	/**
	 * Stop a conversation with the NPC
	 */
	void endTalking(CTrueTalkNPC *npc, bool viewFlag, CViewItem *view = nullptr);

	/**
	 * Start a conversation with the NPC
	 */
	void startTalking(CTrueTalkNPC *npc, uint id, CViewItem *view = nullptr);

	/**
	 * Start a conversation with the NPC
	 */
	void startTalking(const CString &name, uint id, CViewItem *view = nullptr);

	/**
	 * Sets a dial region for a given NPC
	 */
	void talkSetDialRegion(const CString &name, int dialNum, int regionNum);

	/**
	 * Gets a dial region for a given NPC
	 */
	int talkGetDialRegion(const CString &name, int dialNum);

	/*--- CVideoSurface Methods ---*/

	/**
	 * Signal a movie event for the given frame
	 */
	void movieEvent(int frameNumber);

	/**
	 * Signal a movie event at the end of all currently
	 * playing ranges
	 */
	void movieEvent();

	/*--- CGameState Methods ---*/

	void setState1C(bool flag);

	/**
	 * Change to the next season
	 */
	void stateChangeSeason();

	/**
	 * Returns the currently active season
	 */
	Season stateGetSeason() const;

	void stateSet24();
	int stateGet24() const;
	void stateInc38();
	int stateGet38() const;

	/**
	 * Gets the game state node changed counter
	 */
	uint getNodeChangedCtr() const;

	/**
	 * Gets the game state node enter ticks
	 */
	uint getNodeEnterTicks() const;
};

} // End of namespace Titanic

#endif /* TITANIC_GAME_OBJECT_H */
