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

#include "utils.hh"
#include <GEARSystem/worldmap.hh>
#include <utils/freeangles/freeangles.h>
#include <utils/knn/knn.hh>

using namespace WR;
MRCTeam* Utils::_ourTeam = NULL;
MRCTeam* Utils::_theirTeam = NULL;

void Utils::initialize(MRCTeam *ourTeam, MRCTeam *opTeam) {

    kNN::initialize(ourTeam, opTeam);
    FreeAngles::initialize(ourTeam, opTeam);
    _ourTeam = ourTeam;
    _theirTeam = opTeam;
}

Position Utils::vectorSum(const Position &v1, const Position &v2, float m) {
    return Position(true,
                    v1.x() + m*v2.x(),
                    v1.y() + m*v2.y(),
                    v1.z() + m*v2.z());
}

Position Utils::vectorSum(const Position &v1, const Velocity &v2, float m) {
    return Position(true,
                    v1.x() + m*v2.x(),
                    v1.y() + m*v2.y(),
                    v1.z());
}

Position Utils::vectorSum(const Velocity &v1, const Position &v2, float m) {
    return Position(true,
                    v1.x() + m*v2.x(),
                    v1.y() + m*v2.y(),
                    m*v2.z());
}

Velocity Utils::vectorSum(const Velocity &v1, const Velocity &v2, float m) {
    return Velocity(true,
                    v1.x() + m*v2.x(),
                    v1.y() + m*v2.y());
}

Position Utils::threePoints(const Position &near, const Position &far, float distance, float beta) {
    Angle alpha(true, atan2(far.y()-near.y(), far.x()-near.x()));
    Angle gama(true, alpha.value()+beta);
    Position p(true, near.x()+distance*cos(gama.value()), near.y()+distance*sin(gama.value()), 0.0);
    return p;
}

float Utils::getAngle(const Position &a, const Position &b)	{
    return atan2(b.y()-a.y(), b.x()-a.x());
}

float Utils::distance(const Position &a, const Position &b) {
    return sqrt(pow(a.x()-b.x(),2) + pow(a.y()-b.y(),2));
}

float Utils::crossProduct(const Position &A, const Position &B) {
    return A.x()*B.y() - A.y()*B.x();
}

float Utils::scalarProduct(const Position &A, const Position &B) {
    return A.x()*B.x() + A.y()*B.y();
}

float Utils::angleDiff(const Angle &A, const float B) {
    return angleDiff(A.value(), B);
}

float Utils::angleDiff(const float A, const Angle &B) {
    return angleDiff(A, B.value());
}

float Utils::angleDiff(const Angle &A, const Angle &B) {
    return angleDiff(A.value(), B.value());
}

float Utils::angleDiff(const float A, const float B) {
    float diff = fabs(B - A);
    if(diff > GEARSystem::Angle::pi)
        diff = GEARSystem::Angle::twoPi - diff;
    return diff;
}

void Utils::angleLimitZeroTwoPi(float *angle) {
    while(*angle < 0)
        *angle += GEARSystem::Angle::twoPi;
    while(*angle > GEARSystem::Angle::twoPi)
        *angle -= GEARSystem::Angle::twoPi;
}

void Utils::limitValue(float *value, float minValue, float maxValue) {
    if(*value > maxValue)
        *value = maxValue;
    else if(*value < minValue)
        *value = minValue;
}

void Utils::limitMinValue(float *value, float minValue) {
    if(minValue==0)
        return;
    if(*value>0 && *value<minValue)
        *value = minValue;
    else if(*value<0 && *value>-minValue)
        *value = -minValue;
}

bool Utils::checkInterval(double value, double minValue, double maxValue) {
    return (minValue<value && value<maxValue);
}

bool Utils::approximateToZero(float *value, float error){
    if(checkInterval(*value, -error, error)) {
        *value = 0;
        return true;
    } else
        return false;
}


Position Utils::getPlayerKickDevice(quint8 id, float robotRadius) {
    float robotR = robotRadius*2 - 0.03f;

    const Angle ori = _ourTeam->avPlayers().value(id)->orientation();
    float dx = robotR * cos(ori.value());
    float dy = robotR * sin(ori.value());

    const Position pos = _ourTeam->avPlayers().value(id)->position();
    return Position(true, pos.x()+dx, pos.y()+dy, 0.0);
}

bool Utils::isPointAtLine(const Position &s1, const Position &s2, const Position &point) {
    const Position projectedPoint = Utils::projectPointAtLine(s1, s2, point);
    const float dist = Utils::distance(point, projectedPoint);
    return (dist<=0.001f);
}

bool Utils::isPointAtSegment(const Position &s1, const Position &s2, const Position &point) {
    const bool isAtLine = Utils::isPointAtLine(s1, s2, point);
    if(isAtLine==false)
        return false;
    else {
        Position min, max;
        if(s2.x()>=s1.x()) {
            min.setPosition(s1.x(), min.y(), 0.0);
            max.setPosition(s2.x(), max.y(), 0.0);
        } else {
            min.setPosition(s2.x(), min.y(), 0.0);
            max.setPosition(s1.x(), max.y(), 0.0);
        }
        if(s2.y()>=s1.y()) {
            min.setPosition(min.x(), s1.y(), 0.0);
            max.setPosition(max.x(), s2.y(), 0.0);
        } else {
            min.setPosition(min.x(), s2.y(), 0.0);
            max.setPosition(max.x(), s1.y(), 0.0);
        }
        return (point.x()>=min.x() && point.x()<=max.x() && point.y()>=min.y() && point.y()<=max.y());
    }
}

Position Utils::projectPointAtLine(const Position &s1, const Position &s2, const Position &point) {
    const Position a(true, point.x()-s1.x(), point.y()-s1.y(), 0.0);
    const Position b(true, s2.x()-s1.x(), s2.y()-s1.y(), 0.0);
    const float bModule = sqrt(pow(b.x(),2)+pow(b.y(),2));
    const Position bUnitary(true, b.x()/bModule, b.y()/bModule, 0.0);
    const float scalar = Utils::scalarProduct(a, bUnitary);
    return Position(true, s1.x()+scalar*bUnitary.x(), s1.y()+scalar*bUnitary.y(), 0.0);
}

Position Utils::projectPointAtSegment(const Position &s1, const Position &s2, const Position &point) {
    const Position projectedPoint = Utils::projectPointAtLine(s1, s2, point);
    if(Utils::isPointAtSegment(s1, s2, projectedPoint))
        return projectedPoint;
    else {
        const float d1 = Utils::distance(projectedPoint, s1);
        const float d2 = Utils::distance(projectedPoint, s2);
        return (d1<=d2)? s1 : s2;
    }
}

float Utils::distanceToLine(const Position &s1, const Position &s2, const Position &point) {
    const Position projectedPoint = Utils::projectPointAtLine(s1, s2, point);
    const float distance = Utils::distance(point, projectedPoint);
    return (distance<=0.001f)? 0 : distance;
}

float Utils::distanceToSegment(const Position &s1, const Position &s2, const Position &point) {
    const Position projectedPoint = Utils::projectPointAtLine(s1, s2, point);
    if(Utils::isPointAtSegment(s1, s2, projectedPoint))
        return distanceToLine(s1, s2, point);
    else {
        const float d1 = Utils::distance(point, s1);
        const float d2 = Utils::distance(point, s2);
        return (d1<=d2)? d1 : d2;
    }
}

float Utils::getPerpendicularCoefficient(const Position &s1, const Position &s2) {
    float coefAngular= (s2.y()-s1.y())/(s2.x()-s1.x());
    float perpendicularCoef = -(1/coefAngular);
    return perpendicularCoef;
}

Position Utils::hasInterceptionSegments(const Position &s1, const Position &s2, const Position &s3, const Position &s4){
    float denominador = (s4.y()-s3.y())*(s2.x() - s1.x()) - (s4.x() - s3.x())*(s2.y()-s1.y());

    if (denominador!=0.0f){ // segmentos se interceptam
        float u = ((s4.x() - s3.x())*(s1.y() - s3.y()) - (s4.y() - s3.y())*(s1.x() - s3.x()));
        float v = ((s2.x() - s1.x())*(s1.y() - s3.y()) - (s2.y() - s1.y())*(s1.x() - s3.x()));

        if(u !=0.0f){
            float xIntersec = s1.x() + (u/denominador)*(s2.x()-s1.x());
            float yIntersec = s1.y() + (u/denominador)*(s2.y()-s1.y());
            return Position(true, xIntersec, yIntersec, 0.0f);
        }else if(v !=0.0f){
            float xIntersec = s3.x() + (v/denominador)*(s4.x()-s3.x());
            float yIntersec = s3.y() + (v/denominador)*(s4.y()-s3.y());
            return Position(true, xIntersec, yIntersec, 0.0f);
        }else{ // caso especial -> as linhas sao coincidentes
            return Position(true, s1.x(), s1.y(), 0.0f);
        }

    }else{ //Os segmentos sao paralelos
        return Position(false, 0.0f,0.0f,0.0f);
    }
}

std::pair<Position,Position> Utils::getQuadrantPositions(int quadrant) {
    Position initialPos, finalPos;

    // Calc some points
    const float x = fabs(_ourTeam->loc()->ourGoal().x());
    const float y = fabs(_ourTeam->loc()->ourFieldTopCorner().y());

    const Position upL(true, -x, y, 0.0);
    const Position up(true, 0.0, y, 0.0);
    const Position upR(true, x, y, 0.0);
    const Position botL(true, -x, -y, 0.0);
    const Position bot(true, 0.0, -y, 0.0);
    const Position botR(true, x, -y, 0.0);
    const Position cen(true, 0.0, 0.0, 0.0);

    // Set initial position
    if(_ourTeam->loc()->ourSide().isRight()) {
        if(quadrant == QUADRANT_UP) {
            initialPos = up;
        }

        if(quadrant == QUADRANT_UPMID) {
            initialPos = cen;
        }

        if(quadrant == QUADRANT_BOTMID) {
            initialPos = bot;
        }

        if(quadrant == QUADRANT_BOT) {
            initialPos = botL;
        }

    } else {
        if(quadrant == QUADRANT_UP) {
            initialPos = upR;
        }

        if(quadrant == QUADRANT_UPMID) {
            initialPos = up;
        }

        if(quadrant == QUADRANT_BOTMID) {
            initialPos = cen;
        }

        if(quadrant == QUADRANT_BOT) {
            initialPos = bot;
        }
    }

    // Set final position
    if(_ourTeam->loc()->ourSide().isRight()) {
        if(quadrant == QUADRANT_UP) {
            finalPos = upL;
        }

        if(quadrant == QUADRANT_UPMID) {
            finalPos = up;
        }

        if(quadrant == QUADRANT_BOTMID) {
            finalPos = cen;
        }

        if(quadrant == QUADRANT_BOT) {
            finalPos = bot;
        }
    } else {
        if(quadrant == QUADRANT_UP) {
            finalPos = up;
        }

        if(quadrant == QUADRANT_UPMID) {
            finalPos = cen;
        }

        if(quadrant == QUADRANT_BOTMID) {
            finalPos = bot;
        }

        if(quadrant == QUADRANT_BOT) {
            finalPos = botR;
        }
    }

    return std::make_pair(initialPos, finalPos);
}

int Utils::getPlayerQuadrant(Position playerPosition) {
    // Quadrantes relacionados ao campo da divisão B
    if (_ourTeam->loc()->ourSide().isRight()) {
        // QUADRANT_UP
        if (playerPosition.y() < 3.0f && playerPosition.x() > -4.5f && playerPosition.x() < 1.5f * playerPosition.y() - 4.5f) return QUADRANT_UP;
        // QUADRANT_UPMID
        if (playerPosition.y() > 0.0f && playerPosition.x() < 0.0f && playerPosition.x() > 1.5f * playerPosition.y() - 4.5f) return QUADRANT_UPMID;
        // QUADRANT_BOTMID
        if (playerPosition.y() < 0.0f && playerPosition.x() < 0.0f && playerPosition.x() > -1.5f * playerPosition.y() - 4.5f) return QUADRANT_BOTMID;
        // QUADRANT_BOT
        if (playerPosition.y() > -3.0f && playerPosition.x() > -4.5f && playerPosition.x() < -1.5f * playerPosition.y() - 4.5f) return QUADRANT_BOT;
    } else {
        // QUADRANT_UP
        if (playerPosition.y() < 3.0f && playerPosition.x() < 4.5f && playerPosition.x() > -1.5f * playerPosition.y() + 4.5f) return QUADRANT_UP;
        // QUADRANT_UPMID
        if (playerPosition.y() > 0.0f && playerPosition.x() > 0.0f && playerPosition.x() < -1.5f * playerPosition.y() + 4.5f) return QUADRANT_UPMID;
        // QUADRANT_BOTMID
        if (playerPosition.y() < 0.0f && playerPosition.x() > 0.0f && playerPosition.x() < 1.5f * playerPosition.y() + 4.5f) return QUADRANT_BOTMID;
        // QUADRANT_BOT
        if (playerPosition.y() > -3.0f && playerPosition.x() < 4.5f && playerPosition.x() > 1.5f * playerPosition.y() + 4.5f) return QUADRANT_BOT;
    }

    return NO_QUADRANT;
}

int Utils::getPlayerQuadrant(Position playerPosition, FieldSide side) {
    // Quadrantes relacionados ao campo da divisão B
    if (side.isRight()) {
        // QUADRANT_UP
        if (playerPosition.y() < 3.0f && playerPosition.x() > -4.5f && playerPosition.x() < 1.5f * playerPosition.y() - 4.5f) return QUADRANT_UP;
        // QUADRANT_UPMID
        if (playerPosition.y() > 0.0f && playerPosition.x() < 0.0f && playerPosition.x() > 1.5f * playerPosition.y() - 4.5f) return QUADRANT_UPMID;
        // QUADRANT_BOTMID
        if (playerPosition.y() < 0.0f && playerPosition.x() < 0.0f && playerPosition.x() > -1.5f * playerPosition.y() - 4.5f) return QUADRANT_BOTMID;
        // QUADRANT_BOT
        if (playerPosition.y() > -3.0f && playerPosition.x() > -4.5f && playerPosition.x() < -1.5f * playerPosition.y() - 4.5f) return QUADRANT_BOT;
    } else {
        // QUADRANT_UP
        if (playerPosition.y() < 3.0f && playerPosition.x() < 4.5f && playerPosition.x() > -1.5f * playerPosition.y() + 4.5f) return QUADRANT_UP;
        // QUADRANT_UPMID
        if (playerPosition.y() > 0.0f && playerPosition.x() > 0.0f && playerPosition.x() < -1.5f * playerPosition.y() + 4.5f) return QUADRANT_UPMID;
        // QUADRANT_BOTMID
        if (playerPosition.y() < 0.0f && playerPosition.x() > 0.0f && playerPosition.x() < 1.5f * playerPosition.y() + 4.5f) return QUADRANT_BOTMID;
        // QUADRANT_BOT
        if (playerPosition.y() > -3.0f && playerPosition.x() < 4.5f && playerPosition.x() > 1.5f * playerPosition.y() + 4.5f) return QUADRANT_BOT;
    }

    return NO_QUADRANT;
}

int Utils::getOpPlayersInQuadrant(int quadrant) {
    int opPlayersInQuadrant = 0;
    QList<Player*> enemyPlayers = _theirTeam->avPlayers().values();
    QList<Player*>::iterator it;
    for(it = enemyPlayers.begin(); it != enemyPlayers.end(); it++){
        Position playerPosition = (*it)->position();
        int opQuadrant = getPlayerQuadrant(playerPosition);
        if (opQuadrant == quadrant) opPlayersInQuadrant++;
    }
    return opPlayersInQuadrant;
}

Position Utils::getQuadrantBarycenter(int quadrant){
    if(quadrant == NO_QUADRANT) return Position(false, 0.0, 0.0, 0.0);

    std::pair<Position, Position> quadrantPositions = getQuadrantPositions(quadrant);

    float x, y;
    // check if is the same side than ours
    if(_ourTeam->fieldSide().isRight()){
        x = (quadrantPositions.first.x() + quadrantPositions.second.x() + _ourTeam->loc()->theirGoal().x()) / 3.0f;
        y = (quadrantPositions.first.y() + quadrantPositions.second.y() + _ourTeam->loc()->theirGoal().y()) / 3.0f;
    }
    else{
        x = (quadrantPositions.first.x() + quadrantPositions.second.x() + _ourTeam->loc()->ourGoal().x()) / 3.0f;
        y = (quadrantPositions.first.y() + quadrantPositions.second.y() + _ourTeam->loc()->ourGoal().y()) / 3.0f;
    }

    return Position(true, x, y, 0.0);
}
