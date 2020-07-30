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

#ifndef PLAYBOOK_ATTACK_H
#define PLAYBOOK_ATTACK_H

#include <entity/contromodule/playbook/playbook.h>
#include <entity/player/role/mrcroles.h>

class Playbook_Attack : public Playbook {
    Q_OBJECT
private:
    // Roles
    Role_Striker *_rl_stk;
    Role_SecondStriker *_rl_stk2;
    Role_SecondStriker *_rl_stk3;

    // control striker
    bool _takeMainAttacker;
    quint8 mainAttacker;
    int lastNumPlayers;

    void configure(int numPlayers);
    void run(int numPlayers);
    int maxNumPlayer();

    // Mark
    quint8 _attackerId;
    QMutex markMutex;
    QList<int> leftQuadrantList;
    QList<int> rightQuadrantList;
    QList<quint8> markList;
    void resetQuadrantList();
    void resetMarkList();
    quint8 requestMarkPlayer(quint8 playerId);

    // Utils
    bool isBallComing(Position playerPosition, float minVelocity, float radius);

public:
    Playbook_Attack();
    QString name();

signals:
    void sendReceiver(quint8 receiverId);
    void sendAttacker(quint8 attackerId);
    void sendIsMarkNeeded(bool isMarkNeeded);
    void sendQuadrant(int quadrant);

public slots:
    void requestReceivers(quint8 playerId);
    void requestAttacker();
    void requestIsMarkNeeded();
    void requestQuadrant();
};

#endif // PLAYBOOK_ATTACK_H
