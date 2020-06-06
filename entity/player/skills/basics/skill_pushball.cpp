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

#include "skill_pushball.h"
#include <entity/player/skills/skills_include.h>

#define BALLPREVISION_MINVELOCITY 0.02f
#define BALLPREVISION_VELOCITY_FACTOR 3.0f
#define BALLPREVISION_FACTOR_LIMIT 0.15f

QString Skill_PushBall::name() {
    return "Skill_PushBall";
}

Skill_PushBall::Skill_PushBall() {
    _destination.setUnknown();
    setMaxPushDistance(0.8f);
    _state = STATE_POS;
    _currPos.setUnknown();
    _lastPos.setUnknown();
    _distPushed = 0.0f;
}

void Skill_PushBall::run(){
    if(_destination.isUnknown())
        std::cout << "[WARNING]" << name().toStdString() << ": destination not set!\n";

    // Calc behind ball
    Position behindBall = WR::Utils::threePoints(loc()->ball(), _destination, 0.2f, GEARSystem::Angle::pi);

    if(loc()->ballVelocity().abs() > BALLPREVISION_MINVELOCITY){
        // Calc unitary vector of velocity
        const Position velUni(true, loc()->ballVelocity().x()/loc()->ballVelocity().abs(), loc()->ballVelocity().y()/loc()->ballVelocity().abs(), 0.0);

        // Calc velocity factor
        float factor = BALLPREVISION_VELOCITY_FACTOR*loc()->ballVelocity().abs();
        WR::Utils::limitValue(&factor, 0.0f, BALLPREVISION_FACTOR_LIMIT);

        // Calc projected position
        const Position delta(true, factor*velUni.x(), factor*velUni.y(), 0.0);
        Position projectedPos(true, behindBall.x()+delta.x(), behindBall.y()+delta.y(), 0.0);

        if(isBehindBall(projectedPos)){
            behindBall = projectedPos;
        }
    }

    // Local parameters
    Position desiredPos;

    switch (_state) {
    case STATE_POS: {
        player()->dribble(false);
        _currPos.setUnknown();
        _distPushed = 0.0;
        desiredPos = behindBall;

        if(player()->isNearbyPosition(behindBall, 0.03f))
            _state = STATE_PUSH;
    }
        break;
    case STATE_PUSH: {
        player()->dribble(true);
        desiredPos = WR::Utils::threePoints(loc()->ball(), player()->position(), 0.3f, GEARSystem::Angle::pi);

        if(_currPos.isUnknown()){
            _distPushed = 0.0;
            _currPos = loc()->ball();
        }

        _lastPos = _currPos;
        _currPos = loc()->ball();
        _distPushed += WR::Utils::distance(_lastPos, _currPos);

        if(player()->distBall() > 0.35f){
            _state = STATE_POS;
        }

        if(isBallInFront() == false){
            _state = STATE_POS;
        }

        if(_distPushed >= _maxPushDistance){
            player()->idle();
        }
    }
        break;
    }

    // goToLookTo
    Position lookPos = WR::Utils::threePoints(_destination, loc()->ball(), 1000.0f, GEARSystem::Angle::pi);
    player()->goToLookTo(desiredPos, lookPos, 0.01);
}

bool Skill_PushBall::isBehindBall(Position posObjective){
    Position posBall = loc()->ball();
    Position posPlayer = player()->position();
    float anglePlayer = WR::Utils::getAngle(posBall, posPlayer);
    float angleDest = WR::Utils::getAngle(posBall, posObjective);
    float diff = WR::Utils::angleDiff(anglePlayer, angleDest);

    return (diff>GEARSystem::Angle::pi/2.0f);
}

bool Skill_PushBall::isBallInFront(){
    Angle anglePlayerBall = player()->angleTo(loc()->ball());
    float diff = WR::Utils::angleDiff(anglePlayerBall, player()->orientation());

    return (diff <= atan(0.7)); // atan(0.7) aprox = 35 degree
}
