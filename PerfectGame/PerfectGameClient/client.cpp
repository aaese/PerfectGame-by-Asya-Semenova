#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <chrono>
#include "UdpSocket.h"
#include "../GameState/GameState.h"
 
using namespace sf;
 
 
std::string const kIpAddr = "127.0.0.1";
 
u_short const kPort = 8888;
size_t const kBufferSize = 512;
char buffer[kBufferSize];
 
void sleep(unsigned long us)
{
    auto start = std::chrono::high_resolution_clock::now();
    auto finish = std::chrono::high_resolution_clock::now();
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
    while (microseconds.count() < us)
    {
        finish = std::chrono::high_resolution_clock::now();
        microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
    }
}
 
int main(int argc, const char* argv[])
{
    std::unique_ptr<UdpSocket> sock_ptr;
 
    std::cout << "Your name is ";   
    std::string name;
    getline(std::cin,name);
 
    GameState state;
 
    try
    {
        sock_ptr = std::make_unique<UdpSocket>(kIpAddr, kPort);
    }
    catch (std::exception const& err)
    {
        std::cout << "Couldn't init socket: " << err.what() << "\n";
        exit(EXIT_FAILURE);
    }
 
 
    if (sock_ptr->send(name.c_str(), name.length()) != 0)
    {
        std::cout << "Failed to send\n";
        exit(EXIT_FAILURE);
    }
 
    std::cout << "request sent\n";
 
    RenderWindow window(VideoMode(720, 360), "Perfect Game Client");
 
    View view;
    view.reset(FloatRect(0, 0, 720, 360));
 
    Image map_texture;
    map_texture.loadFromFile("../map.png");
    Texture map;
    map.loadFromImage(map_texture);
    Sprite map_sprite;
    map_sprite.setTexture(map);
 
    bool block_was_changed;
    GameIdx block_i;
    GameIdx block_j;
    GameState::Block type_of_block;
 
    Image player_image;
    player_image.loadFromFile("../player.png");
    player_image.createMaskFromColor(Color(0, 0, 255));
    Vector2u image_size = player_image.getSize();
 
    Texture player_texture;
    player_texture.loadFromImage(player_image);
 
    Sprite player_sprite;
    player_sprite.setTexture(player_texture);
    player_sprite.setTextureRect(IntRect(0, 0, image_size.x, image_size.y));
 
    Font font;
    if (!font.loadFromFile("../Miratrix.otf"))
    {
        return 1;
    }
 
    Text player_name(name, font, 20);
    player_name.setFillColor(Color::Black);
    float text_width = player_name.getLocalBounds().width;
 
    RectangleShape select(Vector2f(90, 90));
    select.setFillColor(Color(69, 92, 52, 180));
 
    window.setKeyRepeatEnabled(false);
 
    PlayerPos real_player_position = {2,1};
 
 
    while (1)
    {
        block_was_changed = false;
        size_t sz = kBufferSize;
        if (sock_ptr->recv(buffer, sz) != 0)
        {
            std::cout << "No data to recv\n";
            sleep(1e5);
            continue;
        }
 
        state.deserialize(buffer, sz);
        Player* my_player = state.getPlayer(name);
        my_player->updatePos(real_player_position.first, real_player_position.second);
        PlayerPos now_position = my_player->getPos();
        
                
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::KeyPressed)
            {
                if (event.key.code == Keyboard::A) 
                {   
                    auto pos = my_player->getPos();
                    if ((pos.first != 0) && (state._map[pos.second][pos.first - 1] != GameState::Block::Ground))
                    {
                        my_player->updatePos(pos.first - 1, pos.second);
                    }
                    player_sprite.setTextureRect(IntRect(image_size.x, 0, -(int)image_size.x, image_size.y));
                    pos = my_player->getPos();
                    while (pos.second + 1 < state._map.size() && state._map[pos.second + 1][pos.first] == GameState::Block::Background) {
                        my_player->updatePos(pos.first, pos.second+1);
                        pos = my_player->getPos();
                    }
                }
                else if (event.key.code == Keyboard::D) 
                {
                    auto pos = my_player->getPos();
                    if ((pos.first != state._cols - 1) && (state._map[pos.second][pos.first + 1] != GameState::Block::Ground))
                    {
                        my_player->updatePos(pos.first + 1, pos.second);
                    }
                    player_sprite.setTextureRect(IntRect(0, 0, image_size.x, image_size.y));
                    pos = my_player->getPos();
                    while (pos.second + 1 < state._map.size() && state._map[pos.second+1][pos.first] == GameState::Block::Background) {
                        my_player->updatePos(pos.first, pos.second +1);
                        pos = my_player->getPos();
                    }
                    
                }
                else if (event.key.code == Keyboard::W) 
                {
                    auto pos = my_player->getPos();
                    if ((pos.second != 0) &&
                        (state._map[pos.second - 1][pos.first] != GameState::Block::Ground))
                    {
                        my_player->updatePos(pos.first, pos.second - 1);
                    }
                        
                }
                else if (event.key.code == Keyboard::S) 
                {
                    auto pos = my_player->getPos();
                    if ((pos.second != state._rows - 1) &&
                        (state._map[pos.second + 1][pos.first] != GameState::Block::Ground))
                    {
                        my_player->updatePos(pos.first, pos.second + 1);
                    }                       
                }
            }
 
            else if (event.type == Event::MouseMoved)
            {
                select.setPosition((event.mouseMove.x / 90) * 90, (event.mouseMove.y / 90) * 90);
            }
 
            else if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left)
            {
                block_i = event.mouseButton.x / 90;
                block_j = event.mouseButton.y / 90;
                if (player_sprite.getPosition().x >= (block_i * 90 - 90) && 
                    player_sprite.getPosition().x <= (block_i * 90 + 90) &&
                    player_sprite.getPosition().y >= (block_j * 90 - 90) && 
                    player_sprite.getPosition().y <= (block_j * 90 + 90) &&
                    Vector2f(block_i * 90, block_j * 90) != player_sprite.getPosition())
                {
                    block_was_changed = true;
                    if (state._map[block_j][block_i] == GameState::Block::Background) {
                        state.updateMap(block_j, block_i, GameState::Block::Ground);
                        type_of_block = GameState::Block::Ground;
                    }
                        
                    else
                    {
                        state.updateMap(block_j, block_i, GameState::Block::Background);
                        type_of_block = GameState::Block::Background;
                        auto pos = my_player->getPos();
                        std::cout << (int)pos.second << ' ' << state._map.size() << '\n';
                        if (pos.second < block_j) {
                            while (pos.second+1 < state._map.size() && state._map[pos.second + 1][pos.first] == GameState::Block::Background) {
                                my_player->updatePos(pos.first, pos.second + 1);
                                pos = my_player->getPos();
                            }
                        }
                    }
 
                }
 
            }
 
            else if (event.type == Event::Closed)
            {
                window.close();
                exit(0);
            }
        }
 
        window.clear(sf::Color(66, 170, 255));
 
        for (size_t i = 0; i < state._rows; i++)
            for (size_t j = 0; j < state._cols; j++)
            {
                if (state._map[i][j] == GameState::Block::Ground) {
                    if (i == 2)
                        map_sprite.setTextureRect(IntRect(90, 0, 90, 90));
                    else
                        map_sprite.setTextureRect(IntRect(0, 0, 90, 90));
 
                    map_sprite.setPosition(j * 90, i * 90);
                    window.draw(map_sprite);
                }
            }
 
        Vertex line[] =
        {
            Vertex(Vector2f(0, 0)),
            Vertex(Vector2f(0, 0))
 
        };
 
        line[0].color = Color(117, 156, 89);
        line[1].color = Color(117, 156, 89);
 
        for (size_t i = 0; i < state._map.size(); i++) {
            line[0].position = Vector2f(0, i*90);
            line[1].position = Vector2f(720, i*90);
 
            window.draw(line, 2, Lines);
        }
 
        for (size_t i = 0; i < state._map[0].size(); i++) {
            line[0].position = Vector2f(i*90, 0);
            line[1].position = Vector2f(i*90, 360);
 
            window.draw(line, 2, Lines);
        }
 
        player_sprite.setPosition(my_player->getPos().first * 90, my_player->getPos().second * 90);
        if (my_player->getPos().second != 0)
            player_name.setPosition(my_player->getPos().first * 90 + 45 - text_width / 2, my_player->getPos().second * 90 - 30);
        else
            player_name.setPosition(my_player->getPos().first * 90 + 45 - text_width / 2, 90);
 
        Vector2f pos = player_sprite.getPosition();
 
        if (select.getPosition().x >= (pos.x - 90) && select.getPosition().x <= (pos.x + 90) &&
            select.getPosition().y >= (pos.y - 90) && select.getPosition().y <= (pos.y + 90))
        {
 
            window.draw(select);
        }
 
        player_sprite.setPosition(my_player->getPos().first * 90 + ((90-player_image.getSize().x)/2), my_player->getPos().second * 90);
        window.draw(player_sprite);
        window.draw(player_name);
        player_sprite.setPosition(my_player->getPos().first * 90, my_player->getPos().second * 90);
 
        real_player_position = my_player->getPos();
 
        Sprite other_player;
        other_player.setTexture(player_texture);
        other_player.setColor(Color(255, 255, 255, 155));
        other_player.setTextureRect(IntRect(0, 0, image_size.x, image_size.y));
 
        for (auto& it : state._players) 
        {
            if (it.first != name) 
            {
                other_player.setPosition(it.second.getPos().first * 90, it.second.getPos().second * 90);
                Text other_player_name(it.first, font, 20);
                other_player_name.setFillColor(Color(0, 0, 0, 200));
                other_player_name.setStyle(Text::Bold);
                float width1 = other_player_name.getLocalBounds().width;
                if (it.second.getPos().second != 0)
                    other_player_name.setPosition(it.second.getPos().first * 90 + 45 - width1 / 2, it.second.getPos().second * 90 - 30);
                else
                    other_player_name.setPosition(it.second.getPos().first * 90 + 45 - width1 / 2, 90);
 
                other_player.setPosition(it.second.getPos().first * 90 + ((90 - player_image.getSize().x) / 2), it.second.getPos().second * 90);
                window.draw(other_player_name);
                window.draw(other_player);
                
            }
        }
        window.display();
        
 
        sz = kBufferSize;
        my_player->serialize(buffer, sz);
        if (block_was_changed) 
        {
            buffer[sz++] = block_j;
            buffer[sz++] = block_i;
            buffer[sz++] = (char)type_of_block;
        }
 
        if (sock_ptr->send(buffer, sz) != 0)
        {
            std::cout << "Failed to send pos\n";
            exit(EXIT_FAILURE);
        }
 
    }
 
    return 0;
}
