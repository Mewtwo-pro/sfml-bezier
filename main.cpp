#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector> 
#include <algorithm>            
#include <sstream>
#include <iomanip>
#include <cmath>
sf::RectangleShape createKeyframe(int posx){
    sf::RectangleShape  keyframe(sf::Vector2f(3.f,25.f));
    keyframe.setPosition(posx, 10.f);
    keyframe.setFillColor(sf::Color(222, 120,0));
    return keyframe;
}
struct onionDataValue{
    float x; 
    float y;
};
struct posElipceData{
    float x; 
    float y;
    float frame;
};
int main(){
    float altura = 640.f;
    float ancho = 360.f;
    sf::RenderWindow window(sf::VideoMode(500,750), "ventana");
    sf::RectangleShape drawScena(sf::Vector2f(ancho,altura));
    sf::RectangleShape timelineScena(sf::Vector2f(500.f,60.f));
    drawScena.setFillColor(sf::Color(255, 252, 252));
    drawScena.setOutlineThickness(-2.f);
    drawScena.setOutlineColor(sf::Color::Red);
    drawScena.setPosition(0.f, 0.f);
    timelineScena.setFillColor(sf::Color::Transparent);
    timelineScena.setOutlineThickness(-2.f);
    timelineScena.setOutlineColor(sf::Color::Red);
    sf::RectangleShape current_market(sf::Vector2f(10.f, 30.f)); 
    current_market.setPosition(10.f,10.f);
    
    sf::View timeLineView(sf::FloatRect(0,0, 500.f,60.f));
    timeLineView.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 60.f/750.f));
    std::vector<sf::RectangleShape> listaKeyframes;
    //View
    sf::View drawView(sf::FloatRect(0,0, ancho,altura)); 
    drawView.setViewport(sf::FloatRect(0, 60.f/750.f , ancho/500.f, altura/750.f));
    sf::CircleShape elipce;
    elipce.setFillColor(sf::Color(79, 79, 79));
    elipce.setRadius(50.f);
    elipce.setPosition(10.f, 50.f);
    std::vector<posElipceData> elipceData;
    std::vector<onionDataValue> onionData;
    int k_frame = 1; 
    int end_range = 25;
    bool showOnionSkin = false;
    bool viewListOnionSkin = false;
    sf::CircleShape onionRight(50.f);
    onionRight.setFillColor(
       sf::Color(0, 255 , 0,80)
    ); 
    sf::CircleShape onionLeft(50.f);
    onionLeft.setFillColor(
       sf::Color(255, 0,0,80)
    );
    bool isPlaying = false;
    bool viewMensajeInfo = false;
    bool viewTimeMensaje = false;
    bool blinkKeyframe = false;
    bool isRender = false;
    sf::Clock mensajeClock;
    sf::Clock playClock;
    const float frameTime = 1.f / 24.f;
    sf::Clock durationClock;
        
    float end_onion = 500; 
    float start_onion = 1; 
    float forceValue = 10; 
    sf::Font font;
    if (!font.loadFromFile(
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"))
    {
        std::cout << "No se pudo cargar la fuente\n";
    }
    sf::Text mensaje;
    mensaje.setFont(font);
    mensaje.setString(
        "Atajos de Teclado\n"
        "\n"
        "- Space      : mover al frame anterior\n"
        "- BackSpace  : mover al frame siguiente\n"
        "- K          : guardar keyframe\n"
        "- Enter      : onion skin On\n"
        "- W          : onion skin Off\n"
        "- I          : detener\n"
        "- J          : view list onion\n"
        "- Y          : render\n"
        "- Z          : start onion\n"
        "- F          : blink keyframe\n"
        "- L          : move timeline\n"
        "- X          : end onion\n"
        "- C          : move keyframe right\n"
        "- O          : add force\n"
        "- U          : minus force\n"
    );
    mensaje.setCharacterSize(20);
    mensaje.setFillColor(sf::Color(79, 79, 79));
    mensaje.setPosition(10, 200);
    sf::Text timeMensaje;
    timeMensaje.setFont(font);
    std::stringstream ss;
    ss << "start : " << start_onion;
    std::string txtStarOnion = ss.str();
    timeMensaje.setString(txtStarOnion);
    timeMensaje.setCharacterSize(20);
    timeMensaje.setFillColor(sf::Color(79, 79, 79));
    timeMensaje.setPosition(10, 200);
    while (window.isOpen()){
        sf::Event event;
        while (window.pollEvent(event)){
           if(event.type == sf::Event::Closed)
               window.close();
           if(event.type == sf::Event::KeyPressed){
               if(event.key.code == sf::Keyboard::Space){
                   if (blinkKeyframe){
                       const posElipceData* leftKf = nullptr;
                       float currentFrame = current_market.getPosition().x;
                       for (const auto& kf : elipceData){
                           if (kf.frame < currentFrame){
                               if(!leftKf || kf.frame > leftKf-> frame){
                                   leftKf = &kf;
                                }
                           }
                        }
                        if (leftKf){
                            current_market.setPosition(leftKf->frame, 10);
                            elipce.setPosition(leftKf->x , leftKf->y);
                        }
                       //auto cebolla
                       if (viewListOnionSkin){
                            onionData.clear();
                            float currentFrame = current_market.getPosition().x; 
                            for (const auto& kf : elipceData){
                                if(kf.frame >= start_onion && kf.frame != currentFrame && kf.frame <= end_onion){
                                    onionData.push_back({kf.x , kf.y});
                                }
                            }
                        }
                   }else{
                       auto pos = current_market.getPosition();
                       current_market.setPosition(pos.x-10, 10.f);
                        //set pose 
                       for (const auto& kf : elipceData){
                           auto posKey= kf.frame;
                           if (pos.x - 10== posKey){
                               elipce.setPosition(kf.x , kf.y);
                               break;
                           }
                       }
                   }
               }else if(event.key.code == sf::Keyboard::BackSpace){
                   if (blinkKeyframe){
                       const posElipceData* rightKf = nullptr;
                       float currentFrame = current_market.getPosition().x;
                       for (const auto& kf : elipceData){
                           if (kf.frame > currentFrame){
                               if(!rightKf || kf.frame <  rightKf -> frame){
                                   rightKf = &kf;
                               }
                           }
                       } 
                       if (rightKf){
                           current_market.setPosition(rightKf->frame, 10);
                           elipce.setPosition(rightKf->x , rightKf->y);
                       }
                       //auto cebolla
                       if (viewListOnionSkin){
                            onionData.clear();
                            float currentFrame = current_market.getPosition().x; 
                            for (const auto& kf : elipceData){
                                if(kf.frame >= start_onion && kf.frame != currentFrame && kf.frame <= end_onion){
                                    onionData.push_back({kf.x , kf.y});
                                }
                            }
                        }
                   }else{
                       auto pos = current_market.getPosition();
                       current_market.setPosition(pos.x+10, 10.f);
                        //set pose
                       for (const auto& kf : elipceData){
                           auto posKey= kf.frame;
                           if (pos.x + 10 == posKey){
                               elipce.setPosition(kf.x , kf.y);
                               break;
                           }
                        }
                    }
               }else if (event.key.code == sf::Keyboard::K){
                    auto pos = current_market.getPosition(); 
                    auto posElipce= elipce.getPosition(); 
                    auto it = std::find_if(elipceData.begin(), elipceData.end(),
                        [pos](const posElipceData& p){ return p.frame == pos.x;}
                    ); 
                    if (it != elipceData.end()){
                        it ->x = posElipce.x;
                        it ->y = posElipce.y;
                    } else {
                        listaKeyframes.push_back(createKeyframe(pos.x-1));
                        elipceData.push_back({posElipce.x , posElipce.y , pos.x});
                    }
               }else if (event.key.code == sf::Keyboard::Left){
                    auto pos= elipce.getPosition(); 
                    elipce.setPosition(pos.x - forceValue, pos.y);
               }else if (event.key.code == sf::Keyboard::Right){
                    auto pos= elipce.getPosition(); 
                    elipce.setPosition(pos.x + forceValue, pos.y);
               }else if (event.key.code == sf::Keyboard::Up){
                    auto pos= elipce.getPosition(); 
                    elipce.setPosition(pos.x, pos.y- forceValue);
               }else if (event.key.code == sf::Keyboard::Down){
                    auto pos= elipce.getPosition(); 
                    elipce.setPosition(pos.x, pos.y + forceValue);
               }else if (event.key.code == sf::Keyboard::F){
                    blinkKeyframe = true;
               }else if (event.key.code == sf::Keyboard::C){
                    float currentFrame = current_market.getPosition().x;
                    float offset = 20.f;
                    for (auto& keyf : listaKeyframes){
                        float currentX = keyf.getPosition().x;
                        // Si el frame correspondiente es mayor o igual al actual
                        if ((currentX + 1.f) >= currentFrame){
                            keyf.setPosition(currentX + offset, keyf.getPosition().y);
                        }
                    }
                    for (auto& kf : elipceData){
                        if (kf.frame >= currentFrame){
                            kf.frame += offset;
                        }
                    }
                   if (viewListOnionSkin){
                        onionData.clear();
                        float currentFrame = current_market.getPosition().x; 
                        for (const auto& kf : elipceData){
                            if(kf.frame >= start_onion && kf.frame != currentFrame && kf.frame <= end_onion){
                                onionData.push_back({kf.x , kf.y});
                            }
                        }
                    }
               }else if (event.key.code == sf::Keyboard::U){
                    std::stringstream ss;
                    if (forceValue == 10){
                       forceValue = 50; 
                    }else if(forceValue == 5){
                        forceValue = 10; 
                    }else if(forceValue == 2.5){
                        forceValue =  5; 
                    }
                    ss << "force : " << forceValue; 
                    std::string txtStarOnion = ss.str();
                    timeMensaje.setString(txtStarOnion);
                    viewTimeMensaje = true;
                    mensajeClock.restart();
               }else if (event.key.code == sf::Keyboard::O){
                    std::stringstream ss;
                    if (forceValue == 10){
                       forceValue = 5; 
                    }else if(forceValue == 5){
                        forceValue = 2.5; 
                    }else if(forceValue == 50){
                        forceValue =  10; 
                    }
                    ss << "force : " << forceValue; 
                    std::string txtStarOnion = ss.str();
                    timeMensaje.setString(txtStarOnion);
                    viewTimeMensaje = true;
                    mensajeClock.restart();
               }else if (event.key.code == sf::Keyboard::L){
                    blinkKeyframe = false; 
               }else if (event.key.code == sf::Keyboard::W){
                       showOnionSkin = false;
                       viewListOnionSkin = false;
               }else if (event.key.code == sf::Keyboard::J){
                    viewListOnionSkin = true; 
                    onionData.clear();
                    float currentFrame = current_market.getPosition().x; 
                    for (const auto& kf : elipceData){
                        if(kf.frame >= start_onion && kf.frame != currentFrame && kf.frame <= end_onion){
                            onionData.push_back({kf.x , kf.y});
                        }
                    }
               }else if (event.key.code == sf::Keyboard::X){
                    float currentFrame = current_market.getPosition().x; 
                    end_onion = currentFrame; 
                    std::stringstream ss;
                    ss << "end : " << end_onion;
                    std::string txtStarOnion = ss.str();
                    timeMensaje.setString(txtStarOnion);
                    viewTimeMensaje = true;
                    mensajeClock.restart();
                    
               }else if (event.key.code == sf::Keyboard::Y){
                    isRender = true;
               }else if (event.key.code == sf::Keyboard::Z){
                    float currentFrame = current_market.getPosition().x; 
                    start_onion = currentFrame; 
                    std::stringstream ss;
                    ss << "start : " << start_onion;
                    std::string txtStarOnion = ss.str();
                    timeMensaje.setString(txtStarOnion);
                    viewTimeMensaje = true;
                    mensajeClock.restart();
                    
                    
               }else if (event.key.code == sf::Keyboard::Enter){
                   const posElipceData* leftKf = nullptr;
                   const posElipceData* rightKf = nullptr;
                   float currentFrame = current_market.getPosition().x;
                   for (const auto& kf : elipceData){
                       if (kf.frame < currentFrame){
                           if(!leftKf || kf.frame > leftKf-> frame){
                               leftKf = &kf;
                               onionLeft.setPosition(leftKf->x, leftKf->y);
                            }
                       }
                       if (kf.frame > currentFrame){
                           if(!rightKf || kf.frame <  rightKf -> frame){
                               rightKf = &kf;
                               onionRight.setPosition(rightKf->x, rightKf->y);
                           }
                       }
                   } 
                   showOnionSkin =true;
               }else if (event.key.code == sf::Keyboard::B){
                   isPlaying = true;
                    playClock.restart();
                    durationClock.restart();
               }else if (event.key.code == sf::Keyboard::I){
                   isPlaying = false;
               }else if (event.key.code == sf::Keyboard::F3){
                   if (viewMensajeInfo == false){
                       viewMensajeInfo = true;
                   }else{
                       viewMensajeInfo = false;
                   }
               }
           }
        }
        if (viewTimeMensaje && mensajeClock.getElapsedTime().asSeconds() > 2.f){
            viewTimeMensaje = false;
        }
        if (isPlaying){
            if (playClock.getElapsedTime().asSeconds() >= frameTime){
                playClock.restart();
                auto pos = current_market.getPosition();
                pos.x += 10.f;
                if (pos.x > 10.f + (end_range - 1) * 20.f){
                    current_market.setPosition(10.f,pos.y);
                    isPlaying = false;
                    //float seconds = durationClock.getElapsedTime().asSeconds();
                    //std::cout<<"time :"<<seconds <<std::endl;  
                }else{
                    current_market.setPosition(pos.x , 10.f);
                    for (const auto& kf : elipceData){
                        if (kf.frame == pos.x){
                            elipce.setPosition(kf.x, kf.y);
                            break;
                        }
                    }
                }
            }
        }  
        window.clear(sf::Color::Black);
        window.setView(timeLineView);
        window.draw(timelineScena);
        for (int i = 0; i < end_range; i++){
            if (i > 0 && i < end_range - 1){    
                k_frame = 2;
            }else{
                k_frame = 1;
            }
                
            sf::Vertex vertices[] = {
                sf::Vertex{sf::Vector2f(10.f+ i*20.f,10.f)}, 
                sf::Vertex{sf::Vector2f(10.f+i*20.f,50.f/k_frame)}
            };
            window.draw(vertices, 2,sf::Lines);
        }
        for (const auto& kf : listaKeyframes){
            window.draw(kf);
        }
        window.draw(current_market);
        window.setView(drawView);
        window.draw(drawScena);
        if (viewListOnionSkin){
            for (const auto& kf : onionData){
                sf::CircleShape elipceOnion;
                elipceOnion.setRadius(50.f);
                elipceOnion.setFillColor(sf::Color(90, 90, 90,80));
                elipceOnion.setPosition(kf.x, kf.y);
                window.draw(elipceOnion);
            }
        } 
        if (showOnionSkin){
                window.draw(onionLeft);
                window.draw(onionRight);
        }
        if (viewMensajeInfo){
            window.draw(mensaje); 
        }
        if (viewTimeMensaje){
            window.draw(timeMensaje);
        }
        if (isRender){
            sf::RenderTexture render;
            render.create(ancho, altura);

            int frameCounter = 1;
            float maxFramePosition = 10.f + (end_range - 1) * 20.f;

            for (float framePos = 10.f;
                 framePos <= maxFramePosition;
                 framePos += 10.f)
            {
                // Buscar la posición del keyframe
                for (const auto& kf : elipceData)
                {
                    if (kf.frame == framePos)
                    {
                        elipce.setPosition(kf.x, kf.y);
                        break;
                    }
                }

                // Dibujar únicamente la escena
                render.clear(sf::Color::White);
                render.draw(drawScena);
                render.draw(elipce);
                render.display();

                // Obtener la imagen
                sf::Image img =
                    render.getTexture().copyToImage();

                // Nombre secuencial
                std::ostringstream nombreRender;
                nombreRender << "/home/manuel/Documents/"
                                "cppApps/animation/marker/render/frame"
                             << std::setw(4)
                             << std::setfill('0')
                             << frameCounter
                             << ".png";

                img.saveToFile(nombreRender.str());

                frameCounter++;
            }

            std::cout
                << "Renderizado terminado.\n"
                << "Generando video...\n";

            int resultado = system(
                "ffmpeg -y -framerate 24 "
                "-i /home/manuel/Documents/"
                "cppApps/animation/marker/render/frame%04d.png "
                "-c:v libx264 "
                "-pix_fmt yuv420p "
                "/home/manuel/Documents/"
                "cppApps/animation/marker/animacion.mp4"
            );

            if (resultado == 0){
                timeMensaje.setString("video generado");
                viewTimeMensaje = true;
                mensajeClock.restart();
            }else{
                timeMensaje.setString("error al generar video");
                viewTimeMensaje = true;
                mensajeClock.restart();
            }
            isRender = false;
        }               
        window.draw(elipce);       
        window.display();
                
    }
        return 0;
}
