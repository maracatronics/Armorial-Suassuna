/***
 * Maracatronics Robotics
 * Federal University of Pernambuco (UFPE) at Recife
 * http://www.maracatronics.com/
 *
 * This file is part of Armorial project.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***/

#ifndef SKILL_PLACEMENT_H
#define SKILL_PLACEMENT_H

#include <entity/player/skills/skill.h>

class Skill_Placement : public Skill {
private:
    // Parameters
    Position _destination;

    // State machine
    enum {
        STATE_POS,
        STATE_PUSH,
        STATE_DONE
    };
    int _state;

    void run();
    bool isBehindBall(Position posObjective);
    bool isBallInFront();
public:
    Skill_Placement();
    QString name();
    void setDestination(const Position &destination) { _destination = destination; }
};

#endif // SKILL_PLACEMENT_H
