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

#include "suassuna.h"
#include <entity/player/role/mrcroles.h>
#include <entity/contromodule/strategy/strategy.h>
#include <entity/contromodule/strategy/strategystate.h>
#include <entity/contromodule/strategy/basics/mrcstrategy.h>
#include <entity/player/control/pid.h>
#include <utils/freeangles/freeangles.h>
#include <entity/player/navigation/navalgorithm.h>
#include <entity/player/navigation/fana/fana.h>

Suassuna::Suassuna(quint8 teamId, Colors::Color teamColor, FieldSide fieldSide, bool enableGui, MRCConstants *mrcconstants)
    : _teamId(teamId), _teamColor(teamColor), _fieldSide(fieldSide), _enableGui(enableGui), _mrcconstants(mrcconstants){
    // Create controller
    _ctr = new Controller();

    // Create GUI
    if(_enableGui){
        _ourGUI = new CoachView(_mrcconstants);
    }
    
    // Default field setup
    _defaultField = new Fields::SSL2020();

    // Initialize default values
    this->setServerAddress("localhost", 0);
}

Suassuna::~Suassuna() {
    // Delete controller
    delete _ctr;
    // Delete default field
    delete _defaultField;
}

bool Suassuna::start() {
    // Op team infos parsing
    quint8 opTeamId = (_teamId==0? 1:0);
    Colors::Color opTeamColor = (_teamColor==Colors::YELLOW? Colors::BLUE : Colors::YELLOW);
    FieldSide opFieldSide = (_fieldSide.isRight()? Sides::LEFT : Sides::RIGHT);

    // Server connection
    if(connectToServer()==false)
        return false;

    // Create World
    _world = new World(_ctr, _defaultField, getConstants());

    // Create SSLReferee
    _ref = new SSLReferee();
    _ref->addGameInfo(Colors::YELLOW);
    _ref->addGameInfo(Colors::BLUE);
    _world->addEntity(_ref, 0);

    // Ball moviment detect sensor
    _ballSensor = new BallSensor(_ref, _world->getWorldMap(), getConstants());
    _world->addEntity(_ballSensor, 0);

    // Setup teams
    setupTeams(opTeamId, opTeamColor, opFieldSide);
    _world->setTeams(_ourTeam, _theirTeam);

    // Setup team players
    setupOurPlayers();
    setupOppPlayers(opTeamId);

    // Create coach
    _coach = new Coach(_ref, _ourTeam, _theirTeam, getConstants());
    _world->setControlModule(_coach);

    // Setup strategy for coach
    Strategy *strategy = NULL;
    strategy = new MRCStrategy();
    _coach->setStrategy(strategy);

    // Setup GUI
    if(_enableGui){
        _ourGUI->setTeams(_ourTeam, _theirTeam);
        _ourGUI->setCoach(_coach);
        _ourGUI->setReferee(_ref);
        _world->addEntity(_ourGUI, 2);
    }

    // Setup ball prediction
    /*
    _ballPred = new BallPrediction();
    _ballPred->setTeam(_ourTeam);
    _ballPred->setMinDataSize(50);
    _ballPred->setMinVelocity(0.1);
    _world->addEntity(_ballPred, 2);
    */

    // Initialize utils
    WR::Utils::initialize(_ourTeam, _theirTeam);

    // Initialize MLP
    MLP::loadParameters();

    // Start world
    _world->start();

    return true;
}

void Suassuna::stop() {
    // Stop world and wait for it
    _world->stopEntity();
    _world->wait();

    // Delete teams
    delete _ourTeam;
    delete _theirTeam;

    // Delete modules
    delete _world;
    delete _coach;

    // Disconnect controller
    _ctr->disconnect();
}

void Suassuna::setServerAddress(QString ipAddress, int port) {
    _serverAddress = ipAddress;
    _serverPort = port;
}

bool Suassuna::connectToServer() {
    // Server connection
    if(_ctr->connect(_serverAddress, _serverPort) == false) {
        std::cout << MRCConstants::red << "[ERROR] " << MRCConstants::reset << "failed to connect to GEARSystem's server!";
        return false;
    }
    // Check if GEARSystem's Sensor is connected
    if(_ctr->teams().size() < 2) {
        std::cout << MRCConstants::red << "[ERROR] " << MRCConstants::reset << "no teams on controller, GEARSystem's sensor is not connected!";
        return false;
    }
    return true;
}

void Suassuna::setupTeams(quint8 opTeamId, Colors::Color opTeamColor, FieldSide opFieldSide) {
    // Create teams
    _ourTeam = new MRCTeam(_teamId, _world->getWorldMap(), getConstants());
    _theirTeam = new MRCTeam(opTeamId, _world->getWorldMap(), getConstants());
    // Set opponent teams
    _ourTeam->setOpponentTeam(_theirTeam);
    _theirTeam->setOpponentTeam(_ourTeam);
    // Set our team info
    _ourTeam->setTeamColor(_teamColor);
    _ourTeam->setFieldSide(_fieldSide);
    // Set op team info
    _theirTeam->setTeamColor(opTeamColor);
    _theirTeam->setFieldSide(opFieldSide);
}

void Suassuna::setupOurPlayers() {
    // Create OUR PLAYERS
    QList<quint8> playerList = _world->getWorldMap()->players(_teamId);
    for(quint8 i=0; i<playerList.size() && i<getConstants()->getQtPlayers(); i++) {
        // Create Player
        PID *vxPID = new PID(getConstants()->getLinearKp(), getConstants()->getLinearKi(), getConstants()->getLinearKd(), getConstants()->getRobotMaxLinearSpeed(), -getConstants()->getRobotMaxLinearSpeed());
        PID *vyPID = new PID(getConstants()->getLinearKp(), getConstants()->getLinearKi(), getConstants()->getLinearKd(), getConstants()->getRobotMaxLinearSpeed(), -getConstants()->getRobotMaxLinearSpeed());
        PID *vwPID = new PID(getConstants()->getAngularKp(), getConstants()->getAngularKi(), getConstants()->getAngularKd(), getConstants()->getRobotMaxAngularSpeed(), -getConstants()->getRobotMaxAngularSpeed());
        NavigationAlgorithm *navAlg = new FANA();
        Player *player = new Player(_world, _ourTeam, _ctr, playerList.at(i), new Role_Default(), _ref, vxPID, vyPID, vwPID, navAlg, getConstants());
        // Enable
        player->enable(true);
        // Add to team
        _ourTeam->addPlayer(player);
        // Add to world
        _world->addEntity(player, 2);
        // Enable PID
        player->setPidActivated(true);
    }
}

void Suassuna::setupOppPlayers(quint8 opTeamId) {
    // Create opp. players
    const QList<quint8> opPlayerList = _world->getWorldMap()->players(opTeamId);
    for(quint8 i=0; i<opPlayerList.size() && i<getConstants()->getQtPlayers(); i++) {
        // Create Player
        Player *opPlayer = new Player(_world, _theirTeam, _ctr, opPlayerList.at(i), NULL, _ref, NULL, NULL, NULL, NULL, getConstants());
        // Disable (op team doesnt run)
        opPlayer->enable(false);
        // Add to team
        _theirTeam->addPlayer(opPlayer);
        // Add to world
        _world->addEntity(opPlayer, 2);
    }
}

MRCConstants *Suassuna::getConstants() {
    if(_mrcconstants==NULL)
        std::cout << MRCConstants::red << "[ERROR] " << MRCConstants::reset << "Suassuna" << ", requesting getConstants(), _mrcconstants not initialized!\n";
    return _mrcconstants;
}
