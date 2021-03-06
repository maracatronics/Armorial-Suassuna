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

#include "role_striker.h"

QString Role_Striker::name(){
    return "Role_Striker";
}

Role_Striker::Role_Striker() {
    _bh_atk = NULL;
    _bh_atp = NULL;
    _bh_rcv = NULL;
    _bh_mkb = NULL;
    _bh_mkp = NULL;

    _isMarkNeeded = true;
    _markId = DIST_INVALID_ID;
}

void Role_Striker::initializeBehaviours(){
    usesBehaviour(BEHAVIOUR_ATTACKER,   _bh_atk = new Behaviour_Attacker());
    usesBehaviour(BEHAVIOUR_PENALTYATK, _bh_atp = new Behaviour_Penalty_CF());
    usesBehaviour(BEHAVIOUR_RECEIVER,   _bh_rcv = new Behaviour_Receiver());
    usesBehaviour(BEHAVIOUR_MARKBALL,   _bh_mkb = new Behaviour_MarkBall());
    usesBehaviour(BEHAVIOUR_MARKPLAYER, _bh_mkp = new Behaviour_MarkPlayer());
    usesBehaviour(BEHAVIOUR_PLACEMENT,  _bh_plc = new Behaviour_BallPlacement());
}

void Role_Striker::configure(){
    _config = false;
    setPlacement = false;
}

void Role_Striker::run(){
    SSLGameInfo *gameInfo = ref()->getGameInfo(player()->team()->teamColor());
    if(gameInfo->ourBallPlacement()){
        Position desiredPos = Position(true, ref()->getGameInfo(player()->team()->teamColor())->desiredPosition().x() / 1000.0, ref()->getGameInfo(player()->team()->teamColor())->desiredPosition().y() /  1000.0, 0.0);
        if(!setPlacement){
            _bh_plc->setDesiredPosition(desiredPos);
            setPlacement = true;
        }
        setBehaviour(BEHAVIOUR_PLACEMENT);
    }
    else{
        // Reset placement and threat another ref messages
        setPlacement = false;
        if(gameInfo->penaltyKick()){
            setBehaviour(BEHAVIOUR_PENALTYATK);
        }
        else if(gameInfo->directKick() || gameInfo->indirectKick() || gameInfo->kickoff() || !gameInfo->gameOn()){
            // The striker is our main attacker, so he will make the kick (or be in the line between ball and our goal), or just follow ball
            setBehaviour(BEHAVIOUR_ATTACKER);
        }
        else{
            // game on situation
            emit requestIsMarkNeeded();
            emit requestAttacker();
            //emit requestQuadrant();
            if(_isMarkNeeded){
                if(player()->playerId() == _attackerId){
                    if(!player()->hasBallPossession())
                        setBehaviour(BEHAVIOUR_MARKBALL);
                    else
                        setBehaviour(BEHAVIOUR_ATTACKER);
                }
                else{
                    if(_markId != DIST_INVALID_ID){
                        if(!player()->team()->hasBallPossession()){
                            _bh_mkp->setTargetID(_markId);
                            setBehaviour(BEHAVIOUR_MARKPLAYER);
                        }
                        else{
                            _bh_rcv->setQuadrant(_quadrant);
                            setBehaviour(BEHAVIOUR_RECEIVER);
                        }
                    }
                    else{
                        _bh_rcv->setQuadrant(_quadrant);
                        setBehaviour(BEHAVIOUR_RECEIVER);
                    }
                }
            }
            else{
                if(player()->playerId() == _attackerId){
                    if(isBallComing(STRIKER_INTERCEPT_MINVEL, 1.0f) && (!player()->team()->hasBallPossession() && !player()->team()->opTeam()->hasBallPossession())){
                        _bh_rcv->setQuadrant(_quadrant);
                        setBehaviour(BEHAVIOUR_RECEIVER);
                    }
                    else{
                        if(player()->team()->hasBallPossession() && !player()->hasBallPossession()){
                            _bh_rcv->setQuadrant(_quadrant);
                            setBehaviour(BEHAVIOUR_RECEIVER);
                        }
                        else
                            setBehaviour(BEHAVIOUR_ATTACKER);
                    }
                }
                else{
                    _bh_rcv->setQuadrant(_quadrant);
                    setBehaviour(BEHAVIOUR_RECEIVER);
                }
            }
        }
    }
}

bool Role_Striker::isBallComing(float minVelocity, float radius) {
    const Position posBall = loc()->ball();
    const Position posPlayer = player()->position();

    // Check ball velocity
    if(loc()->ballVelocity().abs() < minVelocity)
        return false;

    // Angle player
    float angVel = loc()->ballVelocity().arg().value();
    float angPlayer = WR::Utils::getAngle(posBall, posPlayer);

    // Check angle difference
    float angDiff = WR::Utils::angleDiff(angVel, angPlayer);
    float angError = atan2(radius, player()->distBall());

    return (fabs(angDiff) < fabs(angError));
}

void Role_Striker::takeAttacker(quint8 attackerId){
    if(_bh_rcv == NULL) return ;
    _attackerId = attackerId;
    _bh_rcv->setAttackerId(attackerId);
}

void Role_Striker::takeIsMarkNeeded(bool isMarkNeeded){
    _isMarkNeeded = isMarkNeeded;
}

void Role_Striker::clearReceivers(){
    if(_bh_atk != NULL) _bh_atk->clearReceivers();
}

void Role_Striker::addReceivers(QList<quint8> rcvList){
    if(_bh_atk != NULL){
        int sz = rcvList.size();
        for(int x = 0; x < sz; x++){
            _bh_atk->addReceiver(rcvList.at(x));
        }
    }
}
