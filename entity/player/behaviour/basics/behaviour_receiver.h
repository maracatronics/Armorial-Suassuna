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

#ifndef BEHAVIOUR_RECEIVER_H
#define BEHAVIOUR_RECEIVER_H

#include <entity/player/behaviour/behaviour.h>
#include <entity/player/skills/skills_include.h>
#include <utils/freeangles/freeangles.h>
#include <QObject>

class Behaviour_Receiver : public Behaviour {
private:
    void configure();
    void run();

    // Skills
    Skill_GoToLookTo *_skill_GoToLookTo;
    Skill_InterceptBall *_skill_Receiver;

    // Parameters
    bool _followAttacker;
    int _quadrant;
    quint8 _attackerId;
    int _state;
    float _minRadius;
    float _maxRadius;

    // Transitions and states enums
    enum {
        STATE_POSITION,
        STATE_RECEIVE,
        STATE_WAIT
    };

    enum{
        SK_GOTO,
        SK_RECV
    };
    // Auxiliary functions
    Position getBestPositionWithoutAttacker(int quadrant);
    QList<FreeAngles::Interval> getGoalFreeAngles(quint8 quadrant, float radius);
    bool isBallComing(float minVelocity, float radius);
    Position getReceiverPosition(int quadrant, quint8 attackerId);

public:
    Behaviour_Receiver();
    QString name();

    // setters
    void setFollowAttacker(bool followAttacker) { _followAttacker = followAttacker; }
    void setAttackerId (int attackerId) {_attackerId = attackerId; }
    void setQuadrant(int quadrant) { _quadrant = quadrant; }
    void setActionRadius(float minRadius, float maxRadius) { _minRadius = minRadius; _maxRadius = maxRadius;}

    // getters
    int getAttackerId(){ return _attackerId; }
    int getQuadrant() { return _quadrant; }
    float getMinRadius() { return _minRadius; }
    float getMaxRadius() { return _maxRadius; }
};

#endif // BEHAVIOUR_RECEIVER_H
