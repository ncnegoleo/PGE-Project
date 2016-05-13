/*
 * Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2016 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LVL_CAMERA_H
#define LVL_CAMERA_H

#include "lvl_base_object.h"
#include <graphics/graphics.h>
#include <common_features/fader.h>
#include <common_features/rectf.h>

#include <luabind/luabind.hpp>
#include <lua_inclues/lua.hpp>

#include <QList>
#include <PGE_File_Formats/file_formats.h>
#include <queue>

class   PGE_Phys_Object;
typedef QVector<PGE_Phys_Object *> PGE_RenderList;

class LVL_Background;
class LVL_Section;

class PGE_LevelCamera
{
    friend class LVL_Background;
    friend class LVL_Section;
    friend class LevelScene;

    //! List of NPC's which needs an activation
    std::queue<PGE_Phys_Object*> npcs_to_activate;
    /*!
     * \brief RTree search callback function which accepts found elements into cache
     * \param item found element
     * \param arg pointer to this camera
     * \return always true
     */
    static bool _TreeSearchCallback(PGE_Phys_Object* item, void* arg);
    /*!
     * \brief Collects elements from vizible area
     * \param zone Area range where look for elements
     */
    void queryItems(PGE_RectF &zone);

    //! Pointer of the parent scene
    LevelScene*  _scene;
public:
    //! Time of one tick in the SMBX Engine (is 1/65)
    static const float _smbxTickTime;
    /*!
     * \brief Constructor
     */
    PGE_LevelCamera(LevelScene *_parent);
    /*!
     * \brief Copy constructor
     * \param cam another camera object
     */
    PGE_LevelCamera(const PGE_LevelCamera &cam);
    /*!
     * \brief Destructor
     */
    ~PGE_LevelCamera();
    /*!
     * \brief Returns width of vizible camera area
     * \return width of vizible camera area
     */
    int w();
    /*!
     * \brief Returns height of vizible camera area
     * \return height of vizible camera area
     */
    int h();
    /*!
     * \brief Returns X position of camera vizible area
     * \return X position of camera vizible area
     */
    double posX();
    /*!
     * \brief Returns Y position of camera vizible area
     * \return Y position of camera vizible area
     */
    double posY();

    float renderX();

    float renderY();

    double centerX();

    double centerY();

    void init(float x, float y, float w, float h);

    void setPos(float x, float y);
    void setRenderPos(float x, float y);
    void setCenterPos(float x, float y);
    void setSize(int w, int h);
    void setOffset(int x, int y);
    void update(float ticks);
    void drawBackground();
    void drawForeground();

    void changeSection(LVL_Section *sct, bool isInit=false);
    void changeSectionBorders(long left, long top, long right, long bottom);
    void resetLimits();

    //PGE_RenderList &renderObjects();
    PGE_Phys_Object**renderObjects_arr();
    int              renderObjects_max();
    int              renderObjects_count();
    void             setRenderObjects_count(int count);
    void             setRenderObjectsCacheEnabled(bool enabled);

    LevelSection* section;
    LVL_Section * cur_section;

    PGE_RectF posRect;

    //! Recent center X position gotten from playable character
    float focus_x;
    //! Recent center Y position gotten from playable character
    float focus_y;

    float render_x;
    float render_y;

    float offset_x;
    float offset_y;

    //! ID of player who owns this camera
    int playerID;

    /**************Fader**************/
    PGE_Fader fader;
    /**************Fader**************/

    /**************Autoscrool**************/
    void resetAutoscroll();
    void processAutoscroll(float tickTime);
    bool  isAutoscroll;
    float _autoscrollVelocityX_max;
    float _autoscrollVelocityY_max;
    float _autoscrollVelocityX;
    float _autoscrollVelocityY;
    PGE_RectF limitBox;
    /**************Autoscrool**************/

    /**************LUA Binding*************/
    static luabind::scope bindToLua();
    /**************************************/

private:
    void _applyLimits();
    void sortElements();
    //PGE_RenderList     objects_to_render;
    PGE_Phys_Object** _objects_to_render;
    int               _objects_to_render_max;
    int               _objects_to_render_stored;
    int               _objects_to_render_recent;
    bool              _disable_cache_mode;
};

#endif // LVL_CAMERA_H
