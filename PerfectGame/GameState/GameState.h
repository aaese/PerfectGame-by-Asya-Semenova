#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "Player.h"

class GameState
{  
public:
    enum class Block { Background = 0, Ground = 1 };
    std::vector<std::vector<Block> > _map;

    GameIdx _rows;
    GameIdx _cols;
    std::unordered_map<std::string, Player> _players;


    GameState(std::string const& config_file_name) {}
    GameState();

    bool serialize(char* buffer, size_t& sz);
    bool deserialize(char const* buffer, size_t const kSize);
    void addPlayer( std::string const& name,
                    std::shared_ptr<UdpSocket> sock,
                    GameIdx x,
                    GameIdx y);
    Player* getPlayer(std::string const& name);
    void incrementAll();
    void sendAll();
    void updateMap(GameIdx x, GameIdx y, Block block);
};