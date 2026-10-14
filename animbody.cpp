#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector> 
#include <algorithm>            
#include <sstream>
#include <iomanip>
#include <cmath>
//salidaa
void setMensajeTime (std:: stringstream& ss , sf::Text& timeMensaje,
    bool& viewTimeMensaje , sf::Clock& mensajeClock){
        timeMensaje.setString(ss.str());
        viewTimeMensaje = true;
        mensajeClock.restart();
    }
sf::RectangleShape createKeyframe(int posx){
    sf::RectangleShape  keyframe(sf::Vector2f(3.f,25.f));
    keyframe.setPosition(posx, 10.f);
    keyframe.setFillColor(sf::Color(222, 120,0));
    return keyframe;
}
struct ctrlRig{
    float x;
    float y;
};
struct onionDataValue{
    float x; 
    float y;
};
sf::VertexArray appendThickVertex(sf::Vector2f p, sf::Vector2f lastP , float thickness, sf::VertexArray outline) {
    sf::Vector2f tangent = p - lastP;
    float length = std::sqrt(tangent.x * tangent.x + tangent.y * tangent.y);
    
    sf::Vector2f normal(0.f, 0.f);
    if (length != 0) {
        normal = sf::Vector2f(-tangent.y / length, tangent.x / length);
    }

    sf::Vector2f pOut = p + normal * (thickness / 2.f);
    sf::Vector2f pIn  = p - normal * (thickness / 2.f);

    outline.append(sf::Vertex(pOut, sf::Color(79, 79 , 79)));
    outline.append(sf::Vertex(pIn, sf::Color(79 , 79 , 79)));
    return outline;
}

sf::Vector2f rotateVector(sf::Vector2f v, float angle) {
    float cosA = std::cos(angle);
    float sinA = std::sin(angle);
    return sf::Vector2f(v.x * cosA - v.y * sinA, v.x * sinA + v.y * cosA);
}
sf::Vector2f calculateBezier(sf::Vector2f p0, sf::Vector2f p1, sf::Vector2f p2, sf::Vector2f p3, float t) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    sf::Vector2f p = uuu * p0;
    p += 3.0f * uu * t * p1;
    p += 3.0f * u * tt * p2;
    p += ttt * p3;
    return p;
}

struct ControlPoint {
    sf::Vector2f pos;
    sf::RectangleShape visualNode;
    
    // Manecillas específicas de este punto
    sf::VertexArray handlesLines;
    std::vector<sf::RectangleShape> handleDots;

    ControlPoint() : handlesLines(sf::Lines) {}

    void initVisual(sf::Color color = sf::Color::Blue) {
        visualNode.setOrigin(6.f, 6.f);
        visualNode.setSize(sf::Vector2f(10,10));
        visualNode.setFillColor(color);
        updateVisualPosition();
    }
    
    void updateVisualPosition() {
        visualNode.setPosition(pos);
    }
};        
        
enum class DrawMode{
    Normal,
    Onion
};

class linealBezier {
public :
    ControlPoint pUp;
    ControlPoint pCenter;
    ControlPoint pBottom;
    ControlPoint* selectedPoint = nullptr;
    sf::VertexArray outline;
    
    bool isSelected = false; // Para saber si toda la línea está seleccionada
    
    // Resto de variables igual...
    float baseRadius = 100.0f; 
    float thickness = 6.0f;
    int pointsPerSegment = 25;
    float kUp = 0.552284f;
    float kCenter = 0.552284f;
    float kBottom = 0.552284f;
    float rotUp = 0.f;
    float rotCenter = 0.f;
    float rotBottom = 0.f;
    float initialAngle = 0.f;
    DrawMode drawMode = DrawMode::Normal;

    linealBezier(sf::Vector2f pStart, sf::Vector2f pEnd) 
    : outline(sf::TriangleStrip) {
        pUp.pos = pStart; 
        pBottom.pos = pEnd;
        pCenter.pos = { (pStart.x + pEnd.x) / 2.f , (pStart.y + pEnd.y) / 2.f };
        
        sf::Vector2f delta = pEnd - pStart;
        baseRadius = std::sqrt(delta.x * delta.x + delta.y * delta.y) * 0.5f; 
        initialAngle = std::atan2(delta.y, delta.x);

        pUp.initVisual();
        pBottom.initVisual();
        pCenter.initVisual();
        selectedPoint = &pBottom;
        
        update();
    }

    void update() {
        outline.clear();
        
        // Limpiar handles individuales antes de recalcular
        pUp.handlesLines.clear();
        pUp.handleDots.clear();
        pCenter.handlesLines.clear();
        pCenter.handleDots.clear();
        pBottom.handlesLines.clear();
        pBottom.handleDots.clear();

        pUp.updateVisualPosition();
        pCenter.updateVisualPosition();
        pBottom.updateVisualPosition();

        sf::Vector2f dir(1.f, 0.f);

        sf::Vector2f upBase     = dir * (baseRadius * kUp);
        sf::Vector2f centerBase = dir * (baseRadius * kCenter);
        sf::Vector2f downBase   = dir * (baseRadius * kBottom);

        sf::Vector2f kUpLeft      = rotateVector(-upBase,     initialAngle + rotUp);
        sf::Vector2f kUpRight     = rotateVector( upBase,     initialAngle + rotUp);
        sf::Vector2f kCenterLeft  = rotateVector(-centerBase, initialAngle + rotCenter);
        sf::Vector2f kCenterRight = rotateVector( centerBase, initialAngle + rotCenter);
        sf::Vector2f kBottomLeft  = rotateVector(-downBase,   initialAngle + rotBottom);
        sf::Vector2f kBottomRight = rotateVector( downBase,   initialAngle + rotBottom);

        sf::Vector2f pUpLeft      = pUp.pos + kUpLeft;
        sf::Vector2f pUpRight     = pUp.pos + kUpRight;
        sf::Vector2f pCenterLeft  = pCenter.pos + kCenterLeft;
        sf::Vector2f pCenterRight = pCenter.pos + kCenterRight;
        sf::Vector2f pBottomLeft  = pBottom.pos + kBottomLeft;
        sf::Vector2f pBottomRight = pBottom.pos + kBottomRight;

        // Asignar manecillas a su respectivo punto de control
        addHandleVisual(pUp, pUp.pos, pUpLeft);
        addHandleVisual(pUp, pUp.pos, pUpRight);

        addHandleVisual(pCenter, pCenter.pos, pCenterLeft);
        addHandleVisual(pCenter, pCenter.pos, pCenterRight);

        addHandleVisual(pBottom, pBottom.pos, pBottomLeft);
        addHandleVisual(pBottom, pBottom.pos, pBottomRight);

        // Curva Bezier y contorno (igual que antes)
        std::vector<sf::Vector2f> curvePoints;
        for (int i = 0; i <= pointsPerSegment; i++) {
            float t = (float)i / pointsPerSegment;
            curvePoints.push_back(calculateBezier(pUp.pos, pUpRight, pCenterLeft, pCenter.pos, t));
        }
        for (int i = 1; i <= pointsPerSegment; i++) {
            float t = (float)i / pointsPerSegment;
            curvePoints.push_back(calculateBezier(pCenter.pos, pCenterRight, pBottomLeft, pBottom.pos, t));
        }

        for (size_t i = 0; i < curvePoints.size(); i++) {
            sf::Vector2f lastP = (i == 0) ? (curvePoints[0] + (curvePoints[0] - curvePoints[1])) 
                                          : curvePoints[i - 1];
            outline = appendThickVertex(curvePoints[i], lastP, thickness, outline);
        }
    }

    void addHandleVisual(ControlPoint& cp, sf::Vector2f origin, sf::Vector2f handlePos) {
        cp.handlesLines.append(sf::Vertex(origin, sf::Color(150, 150, 150, 200)));
        cp.handlesLines.append(sf::Vertex(handlePos, sf::Color(150, 150, 150, 200)));
        
        sf::RectangleShape dot;
        dot.setOrigin(4.f, 4.f);
        dot.setSize(sf::Vector2f(5.f, 5.f));
        dot.setPosition(handlePos);
        dot.setFillColor(sf::Color(0, 180, 255));
        cp.handleDots.push_back(dot);
    }

    void draw(sf::RenderTarget& target) {
        if (drawMode == DrawMode::Normal){
            // La curva del cuerpo siempre se muestra
            target.draw(outline);

            if (isSelected) {
                // Dibujar solo los elementos del punto que esté seleccionado actualmente
                if (selectedPoint == &pUp) {
                    target.draw(pUp.handlesLines);
                    for (auto& dot : pUp.handleDots) target.draw(dot);
                    target.draw(pUp.visualNode);
                } 
                else if (selectedPoint == &pCenter) {
                    target.draw(pCenter.handlesLines);
                    for (auto& dot : pCenter.handleDots) target.draw(dot);
                    target.draw(pCenter.visualNode);
                } 
                else if (selectedPoint == &pBottom) {
                    target.draw(pBottom.handlesLines);
                    for (auto& dot : pBottom.handleDots) target.draw(dot);
                    target.draw(pBottom.visualNode);
                }
            }
        }else if (drawMode == DrawMode::Onion){
            sf::VertexArray onionOutline = outline;
            for (size_t i = 0; i< onionOutline.getVertexCount(); ++i){
                onionOutline[i].color = sf::Color(120, 120 , 120 , 80);
            }
            target.draw(onionOutline);
        }
    }
};

void selectCtrl(std::vector<linealBezier>& fullbody, linealBezier*& selectedLinea, linealBezier*& preSelect, ControlPoint* selPoint) {
    // Desactivar la selección en todas las partes del cuerpo
    for (auto& part : fullbody) {
        part.isSelected = false;
        part.pUp.visualNode.setFillColor(sf::Color::Blue);
        part.pCenter.visualNode.setFillColor(sf::Color::Blue);
        part.pBottom.visualNode.setFillColor(sf::Color::Blue);
    }

    // Activar la línea y el punto específico
    selectedLinea->isSelected = true;
    selectedLinea->selectedPoint = selPoint;

    // Pintar de rojo únicamente el nodo seleccionado
    selPoint->visualNode.setFillColor(sf::Color::Red);
    preSelect = selectedLinea;
}

struct posElipceData{
    float x; 
    float y;
    float frame;
};
struct keyNodeBezier
{
    float frame;

    float pUp_x;
    float pUp_y;
    float up_rot;
    float up_tangent;

    float mid_x;
    float mid_y;
    float mid_rot;
    float mid_tangent;

    float dow_x;
    float dow_y;
    float dow_rot;
    float dow_tangent;
};
// PIN: definir struct keyframe    
struct bodyKeyframe
{
    float frame;
    std::vector<keyNodeBezier> parts;

    void capture(float frameNumber,
                 const std::vector<linealBezier>& fullbody)
    {
        frame = frameNumber;
        parts.clear();

        for (const auto& bodyPart : fullbody)
        {
            parts.push_back({
                frameNumber,
                bodyPart.pUp.pos.x,
                bodyPart.pUp.pos.y,
                bodyPart.rotUp,
                bodyPart.kUp,

                bodyPart.pCenter.pos.x,
                bodyPart.pCenter.pos.y,
                bodyPart.rotCenter,
                bodyPart.kCenter,

                bodyPart.pBottom.pos.x,
                bodyPart.pBottom.pos.y,
                bodyPart.rotBottom,
                bodyPart.kBottom
            });
        }
    }
};
    
void applyBodyKeyframe(const bodyKeyframe& key,std::vector<linealBezier>& fullbody){
    for (size_t i = 0; i < fullbody.size(); ++i)
    {
        const auto& src = key.parts[i];
        auto& dst = fullbody[i];

        dst.pUp.pos = {src.pUp_x, src.pUp_y};
        dst.pCenter.pos = {src.mid_x, src.mid_y};
        dst.pBottom.pos = {src.dow_x, src.dow_y};

        dst.rotUp = src.up_rot;
        dst.kUp = src.up_tangent;

        dst.rotCenter = src.mid_rot;
        dst.kCenter = src.mid_tangent;

        dst.rotBottom = src.dow_rot;
        dst.kBottom = src.dow_tangent;

        dst.update();
    }
}
void autoSaveKeyframe(sf::RectangleShape& current_market, 
                      sf::CircleShape& elipce, 
                      std::vector<sf::RectangleShape>& listaKeyframes, 
                      std::vector<posElipceData>& elipceData, 
                      std::vector<bodyKeyframe>& keyPartData, 
                      const std::vector<linealBezier>& fullbody) 
{
    auto pos = current_market.getPosition(); 
    auto posElipce = elipce.getPosition(); 
    
    // Buscar si ya existe la elipse en este frame
    auto it = std::find_if(elipceData.begin(), elipceData.end(),
        [pos](const posElipceData& p){ return p.frame == pos.x;}
    ); 

    // Buscar si ya existe el bodyKeyframe en este frame
    auto kBody = std::find_if(
        keyPartData.begin(),
        keyPartData.end(),
        [pos](const bodyKeyframe& k)
        {
            return k.frame == pos.x;
        });

    // Actualizar o insertar el keyframe del cuerpo
    if (kBody != keyPartData.end()){
        kBody->capture(pos.x, fullbody);
    } else {
        bodyKeyframe key;
        key.capture(pos.x, fullbody);
        keyPartData.push_back(std::move(key));
    }

    // Actualizar o insertar el keyframe de la elipse
    if (it != elipceData.end()){
        it->x = posElipce.x;
        it->y = posElipce.y;
    } else {
        listaKeyframes.push_back(createKeyframe(pos.x - 1));
        elipceData.push_back({posElipce.x, posElipce.y, pos.x});
    }
}
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
    std::vector<sf::RectangleShape> listaKeyBody;
    //View
    sf::View drawView(sf::FloatRect(0,0, ancho,altura)); 
    drawView.setViewport(sf::FloatRect(0, 60.f/750.f , ancho/500.f, altura/750.f));
    sf::CircleShape elipce;
    elipce.setFillColor(sf::Color(79, 79, 79));
    elipce.setRadius(50.f);
    // PIN: definir lista keyframe de objetos creados en la escena 
    std::vector<posElipceData> elipceData;
    std::vector<onionDataValue> onionData;
    std::vector<bodyKeyframe> onionBodyData;
    std::vector<posElipceData> headData;
    std::vector<bodyKeyframe> keyPartData;
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
    // PIN: atajos info l
        "- Y          : hacer Render\n"
        "- K          : create keyfreme\n"
        "- X          : rotate ctrl\n"
        "- E          : estirar ctrl\n"
        "- R          : move ctrl\n"
        "- Q          : hide onion skin\n"
        "- P          : sel ArmLeft\n"
        "- O          : sel ArmRight\n"
        "- U          : sel LegLeft\n"
        "- N          : sel LegRight\n"
        "- M          : sel spine\n"
        "- D          : sel up\n"
        "- S          : sel mid\n"
        "- A          : sel bottom\n"
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
        
    // componentes body 
    
    float tangentStep = 0.05f;
    float rotationStep = 0.05f; // ⬅️ Velocidad de giro (en radianes) de la manecilla
    ctrlRig upSpineCtrl;
    upSpineCtrl.x = 300.f;         
    upSpineCtrl.y = 100.f;
    elipce.setPosition(upSpineCtrl.x-50, upSpineCtrl.y- 100);
    linealBezier* selectedLinea= nullptr;
    // PIN: creacion body curve bezier
    linealBezier spine = linealBezier(sf::Vector2f(upSpineCtrl.x , upSpineCtrl.y) , 
        sf::Vector2f(upSpineCtrl.x , upSpineCtrl.y + 100));
    linealBezier ArmLeft(sf::Vector2f(upSpineCtrl.x , upSpineCtrl.y) , 
        sf::Vector2f(upSpineCtrl.x - 50, upSpineCtrl.y + 50));
    linealBezier ArmRight(sf::Vector2f(upSpineCtrl.x , upSpineCtrl.y) , 
        sf::Vector2f(upSpineCtrl.x + 50, upSpineCtrl.y + 50));
    linealBezier LegLeft(sf::Vector2f(upSpineCtrl.x , upSpineCtrl.y + 100) , 
        sf::Vector2f(upSpineCtrl.x - 50, upSpineCtrl.y + 150));
    linealBezier LegRight(sf::Vector2f(upSpineCtrl.x , upSpineCtrl.y + 100) , 
        sf::Vector2f(upSpineCtrl.x + 50, upSpineCtrl.y + 150));
    std::vector <linealBezier> fullbody;
    fullbody.push_back(ArmLeft);
    fullbody.push_back(ArmRight);
    fullbody.push_back(LegLeft);
    fullbody.push_back(LegRight);
    fullbody.push_back(spine);
    int movMode = 4;
    bool movEjeX = true;
    bool hipMove = false;
    bool autoKeyEnabled = false;
    linealBezier* preSelect = &fullbody[4];
    selectedLinea = &fullbody[4];   // Spine
    selectedLinea->isSelected = true;
    selectedLinea->selectedPoint = &selectedLinea->pBottom;
    selectedLinea->pCenter.visualNode.setFillColor(sf::Color::Blue);
    selectedLinea->pUp.visualNode.setFillColor(sf::Color::Blue);
    selectedLinea->pBottom.visualNode.setFillColor(sf::Color::Red);
    while (window.isOpen()){
        sf::Event event;
        while (window.pollEvent(event)){
            if(event.type == sf::Event::Closed)
               window.close();
            if(event.type == sf::Event::KeyPressed){
                if(event.key.code == sf::Keyboard::Space){
                    if (movMode == 4){
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
                                onionBodyData.clear();
                                float currentFrame = current_market.getPosition().x; 
                                for (const auto& kf : elipceData){
                                    if(kf.frame >= start_onion && kf.frame != currentFrame && kf.frame <= end_onion){
                                        onionData.push_back({kf.x , kf.y});
                                    }
                                }
                                for (const auto& bkf : keyPartData){
                                    if (bkf.frame >= start_onion && bkf.frame != currentFrame && bkf.frame <= end_onion){
                                        onionBodyData.push_back(bkf);
                                    }
                                }
                           }
                        }else{
                            auto pos = current_market.getPosition();
                            current_market.setPosition(pos.x-10, 10.f);
                             // PIN: move single in timeline  
                            for (const auto& kf : elipceData){
                                auto posKey= kf.frame;
                                if (pos.x - 10== posKey){
                                    elipce.setPosition(kf.x , kf.y);
                                    break;
                                }
                            }
                            auto bodyIt = std::find_if(
                                keyPartData.begin(),
                                keyPartData.end(),
                                [pos](const bodyKeyframe& k)
                                {
                                    return k.frame == pos.x-10;
                                });

                            if (bodyIt != keyPartData.end())
                            {
                                applyBodyKeyframe(*bodyIt, fullbody);
                            }
                        }
                    }else if (movMode == 1){
                        if (movEjeX == true){
                            selectedLinea->selectedPoint->pos.x += forceValue; 
                            if (selectedLinea == &fullbody[4]){
                                if (selectedLinea->selectedPoint == &(selectedLinea->pBottom)){
                                    fullbody[2].pUp.pos.x += forceValue;
                                    fullbody[2].update();
                                    fullbody[3].pUp.pos.x += forceValue;
                                    fullbody[3].update();
                                }else if (selectedLinea->selectedPoint == &(selectedLinea->pUp)){
                                    fullbody[0].pUp.pos.x += forceValue;
                                    fullbody[0].update();
                                    fullbody[1].pUp.pos.x += forceValue;
                                    fullbody[1].update();                            
                                    auto pos= elipce.getPosition(); 
                                    elipce.setPosition(pos.x + forceValue, pos.y);
                                }
                            }
                        }else {
                           selectedLinea->selectedPoint->pos.y += forceValue; 
                            if (selectedLinea == &fullbody[4]){
                                if (selectedLinea->selectedPoint == &(selectedLinea->pBottom)){
                                    fullbody[2].pUp.pos.y += forceValue;
                                    fullbody[2].update();
                                    fullbody[3].pUp.pos.y += forceValue;
                                    fullbody[3].update();
                                }else if (selectedLinea->selectedPoint == &(selectedLinea->pUp)){
                                    fullbody[0].pUp.pos.y += forceValue;
                                    fullbody[0].update();
                                    fullbody[1].pUp.pos.y += forceValue;
                                    fullbody[1].update();                            
                                    auto pos= elipce.getPosition(); 
                                    elipce.setPosition(pos.x, pos.y + forceValue);
                                }
                            }
                        }
                        selectedLinea->update();
                        if (autoKeyEnabled){
                            autoSaveKeyframe(current_market , elipce , listaKeyframes ,
                                elipceData, keyPartData, fullbody
                            );
                        }                      
                    }else if (movMode == 2){
                        //rot handless
                        if (selectedLinea->selectedPoint == &selectedLinea->pUp){
                            selectedLinea->rotUp += rotationStep;
                        }else if (selectedLinea->selectedPoint == &selectedLinea->pCenter){
                            selectedLinea->rotCenter+= rotationStep;
                        }else if (selectedLinea->selectedPoint == &selectedLinea->pBottom){
                            selectedLinea->rotBottom+= rotationStep;
                        }
                        selectedLinea->update();
                        
                        if (autoKeyEnabled){
                            autoSaveKeyframe(current_market , elipce , listaKeyframes ,
                                elipceData, keyPartData, fullbody
                            );
                        }                      
                    }else if (movMode == 3){
                        if (selectedLinea->selectedPoint == &selectedLinea->pUp){
                            selectedLinea->kUp += tangentStep;
                        }else if (selectedLinea->selectedPoint == &selectedLinea->pCenter){
                            selectedLinea->kCenter+= tangentStep;
                        }else if (selectedLinea->selectedPoint == &selectedLinea->pBottom){
                            selectedLinea->kBottom+= tangentStep;
                        }
                        selectedLinea->update();
                        if (autoKeyEnabled){
                            autoSaveKeyframe(current_market , elipce , listaKeyframes ,
                                elipceData, keyPartData, fullbody
                            );
                        }                      
                    }
                }else if(event.key.code == sf::Keyboard::BackSpace){
                    if (movMode == 4){
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
                        
                             auto bodyIt = std::find_if(
                                 keyPartData.begin(),
                                 keyPartData.end(),
                                 [pos](const bodyKeyframe& k)
                                 {
                                     return k.frame == pos.x+10;
                                 });

                             if (bodyIt != keyPartData.end())
                             {
                                 applyBodyKeyframe(*bodyIt, fullbody);
                             }
                        }
                    }else if (movMode == 1){
                        if (movEjeX == true){
                            selectedLinea->selectedPoint->pos.x -= forceValue; 
                            if (selectedLinea == &fullbody[4]){
                                if (selectedLinea->selectedPoint == &(selectedLinea->pBottom)){
                                    fullbody[2].pUp.pos.x -= forceValue;
                                    fullbody[2].update();
                                    fullbody[3].pUp.pos.x -= forceValue;
                                    fullbody[3].update();
                                }else if (selectedLinea->selectedPoint == &(selectedLinea->pUp)){
                                    fullbody[0].pUp.pos.x -= forceValue;
                                    fullbody[0].update();
                                    fullbody[1].pUp.pos.x -= forceValue;
                                   fullbody[1].update();                            
                                    auto pos= elipce.getPosition(); 
                                    elipce.setPosition(pos.x- forceValue, pos.y);
                                }
                            }
                        }else {
                            selectedLinea->selectedPoint->pos.y -= forceValue; 
                            if (selectedLinea == &fullbody[4]){
                                if (selectedLinea->selectedPoint == &(selectedLinea->pBottom)){
                                    fullbody[2].pUp.pos.y -= forceValue;
                                    fullbody[2].update();
                                    fullbody[3].pUp.pos.y -= forceValue;
                                    fullbody[3].update();
                                }else if (selectedLinea->selectedPoint == &(selectedLinea->pUp)){
                                    fullbody[0].pUp.pos.y -= forceValue;
                                    fullbody[0].update();
                                    fullbody[1].pUp.pos.y -= forceValue;
                                    fullbody[1].update();                            
                                    auto pos= elipce.getPosition(); 
                                    elipce.setPosition(pos.x, pos.y - forceValue);
                                }
                            }
                        }
                        selectedLinea->update();
                    }else if (movMode == 2){
                        //rot handless
                        if (selectedLinea->selectedPoint == &selectedLinea->pUp){
                            selectedLinea->rotUp -= rotationStep;
                        }else if (selectedLinea->selectedPoint == &selectedLinea->pCenter){
                            selectedLinea->rotCenter-= rotationStep;
                        }else if (selectedLinea->selectedPoint == &selectedLinea->pBottom){
                            selectedLinea->rotBottom-= rotationStep;
                        }
                        selectedLinea->update();
                    }else if (movMode == 3){
                        if (selectedLinea->selectedPoint == &selectedLinea->pUp){
                            selectedLinea->kUp = std::max(0.f , selectedLinea->kUp - tangentStep);
                        }else if (selectedLinea->selectedPoint == &selectedLinea->pCenter){
                            selectedLinea->kCenter= std::max(0.f , selectedLinea->kCenter- tangentStep);
                        }else if (selectedLinea->selectedPoint == &selectedLinea->pBottom){
                            selectedLinea->kBottom= std::max(0.f , selectedLinea->kBottom- tangentStep);
                        }
                        selectedLinea->update();
                    }
                }else if (event.key.code == sf::Keyboard::K){
                    // PIN: set create keyframe
                    auto pos = current_market.getPosition(); 
                    auto posElipce= elipce.getPosition(); 
                    auto it = std::find_if(elipceData.begin(), elipceData.end(),
                        [pos](const posElipceData& p){ return p.frame == pos.x;}
                    ); 
                    auto kBody = std::find_if(
                        keyPartData.begin(),
                        keyPartData.end(),
                        [pos](const bodyKeyframe& k)
                        {
                            return k.frame == pos.x;
                        });
                    if (kBody != keyPartData.end()){
                        kBody->capture(pos.x, fullbody);
                    }else{
                        bodyKeyframe key;
                        key.capture(pos.x, fullbody);
                        keyPartData.push_back(std::move(key));
                    }
                    //si ya existe un key frame lo actualiza
                    if (it != elipceData.end()){
                        it ->x = posElipce.x;
                        it ->y = posElipce.y;
                    //si no existe aun guarda uno nuevo
                    } else {
                        listaKeyframes.push_back(createKeyframe(pos.x-1));
                        elipceData.push_back({posElipce.x , posElipce.y , pos.x});
                    }
                }else if (event.key.code == sf::Keyboard::Left){
                    auto pos= elipce.getPosition(); 
                    elipce.setPosition(pos.x - forceValue, pos.y);
                    for (auto& bodyPart : fullbody){
                        bodyPart.pUp.pos.x -= forceValue; 
                        bodyPart.pCenter.pos.x -= forceValue;   // Mueve también el cen
                        bodyPart.pBottom.pos.x -= forceValue;  // Mueve también el final
                        bodyPart.update(); 
                    }
                    if (autoKeyEnabled){
                        autoSaveKeyframe(current_market , elipce , listaKeyframes ,
                            elipceData, keyPartData, fullbody
                        );
                    }
                }else if (event.key.code == sf::Keyboard::Right){
                    auto pos= elipce.getPosition(); 
                    elipce.setPosition(pos.x + forceValue, pos.y);
                    for (auto& bodyPart : fullbody){
                        bodyPart.pUp.pos.x += forceValue; 
                        bodyPart.pCenter.pos.x += forceValue;   // Mueve también el centro
                        bodyPart.pBottom.pos.x += forceValue;  // Mueve también el final
                        bodyPart.update(); 
                    }
                    if (autoKeyEnabled){
                        autoSaveKeyframe(current_market , elipce , listaKeyframes ,
                            elipceData, keyPartData, fullbody
                        );
                    }
                }else if (event.key.code == sf::Keyboard::Up){
                    auto pos= elipce.getPosition(); 
                    elipce.setPosition(pos.x, pos.y- forceValue);
                    for (auto& bodyPart : fullbody){
                        bodyPart.pUp.pos.y -= forceValue; 
                        bodyPart.pCenter.pos.y -= forceValue;   // Mueve también el centro
                        bodyPart.pBottom.pos.y -= forceValue;  // Mueve también el final
                        bodyPart.update(); 
                    }
                    if (autoKeyEnabled){
                        autoSaveKeyframe(current_market , elipce , listaKeyframes ,
                            elipceData, keyPartData, fullbody
                        );
                    }
                }else if (event.key.code == sf::Keyboard::Down){
                     auto pos= elipce.getPosition(); 
                     elipce.setPosition(pos.x, pos.y + forceValue);
                      // Actualizar cada parte del cuerpo y forzar su recalculo
                      for (auto& bodyPart : fullbody){
                          bodyPart.pUp.pos.y += forceValue; 
                          bodyPart.pCenter.pos.y += forceValue;   // Mueve también el centro
                          bodyPart.pBottom.pos.y += forceValue;  // Mueve también el final
                         
                          bodyPart.update(); // 💡 CRUCIAL: Aplica los cambios geométricos
                      }
                    if (autoKeyEnabled){
                        autoSaveKeyframe(current_market , elipce , listaKeyframes ,
                            elipceData, keyPartData, fullbody
                        );
                    }                      
                    
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
                }else if (event.key.code == sf::Keyboard::V){
                     std::stringstream ss;
                     if (forceValue == 10){
                        forceValue = 50; 
                     }else if(forceValue == 5){
                         forceValue = 10; 
                     }else if(forceValue == 2.5){
                         forceValue =  5; 
                     }
                     rotationStep = 0.005f * forceValue;
                     tangentStep = 0.005f * forceValue;
                     ss << "force : " << forceValue; 
                     setMensajeTime(ss, timeMensaje , viewTimeMensaje, mensajeClock);
                }else if (event.key.code == sf::Keyboard::T){
                     std::stringstream ss;
                     if (forceValue == 10){
                        forceValue = 5; 
                     }else if(forceValue == 5){
                         forceValue = 2.5; 
                     }else if(forceValue == 50){
                         forceValue =  10; 
                     }
                     rotationStep = 0.005f * forceValue;
                     tangentStep = 0.005f * forceValue;
                     ss << "force : " << forceValue; 
                     setMensajeTime(ss, timeMensaje , viewTimeMensaje, mensajeClock);
                }else if (event.key.code == sf::Keyboard::L){
                     blinkKeyframe = false; 
                }else if (event.key.code == sf::Keyboard::Q){
                        showOnionSkin = false;
                        viewListOnionSkin = false;
                }else if (event.key.code == sf::Keyboard::J){
                    viewListOnionSkin = true; 
                    onionData.clear();
                    onionBodyData.clear();
                    
                    float currentFrame = current_market.getPosition().x; 
                    for (const auto& kf : elipceData){
                        if(kf.frame >= start_onion && kf.frame != currentFrame && kf.frame <= end_onion){
                            onionData.push_back({kf.x , kf.y});
                        }
                    }
                    for (const auto& bkf : keyPartData){
                        if (bkf.frame >= start_onion && bkf.frame != currentFrame && bkf.frame <= end_onion){
                            onionBodyData.push_back(bkf);
                        }
                    }
                }else if (event.key.code == sf::Keyboard::H){
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
                }else if (event.key.code == sf::Keyboard::W){
                    movMode = 2;
                }else if (event.key.code == sf::Keyboard::E){
                    movMode = 3;
                }else if (event.key.code == sf::Keyboard::R){
                    movMode = 1;
                }else if (event.key.code == sf::Keyboard::X){
                    movMode = 4;
                    
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
                }else if (event.key.code == sf::Keyboard::F2){
                    if (autoKeyEnabled == false){
                        autoKeyEnabled = true;
                        ss.str("");
                        ss.clear();
                        ss<<"autoKey : " << "activado"; 
                        setMensajeTime(ss, timeMensaje , viewTimeMensaje, mensajeClock);
                    }else{
                        autoKeyEnabled = false;
                        ss.str("");
                        ss.clear();
                        ss << "autoKey : " << "desactivado"; 
                        setMensajeTime(ss, timeMensaje , viewTimeMensaje, mensajeClock);
                    }
                }else if (event.key.code == sf::Keyboard::D){
                    selectedLinea = &fullbody[4];
                    selectCtrl(fullbody,selectedLinea,preSelect,&selectedLinea->pUp);
                }else if (event.key.code == sf::Keyboard::S){
                    selectCtrl(fullbody,selectedLinea,preSelect,&selectedLinea->pCenter);
                }else if (event.key.code == sf::Keyboard::P){
                    //ArmLeft
                    selectedLinea = &fullbody[0];
                    selectCtrl(fullbody,selectedLinea,preSelect,&selectedLinea->pBottom);
                }else if (event.key.code == sf::Keyboard::O){
                    //ArmRight
                    selectedLinea = &fullbody[1];
                    selectCtrl(fullbody,selectedLinea,preSelect,&selectedLinea->pBottom);
                }else if (event.key.code == sf::Keyboard::U){
                    //LegLeft
                    selectedLinea = &fullbody[2];
                    selectCtrl(fullbody,selectedLinea,preSelect,&selectedLinea->pBottom);
                }else if (event.key.code == sf::Keyboard::N){
                    //LegRight
                    selectedLinea = &fullbody[3];
                    selectCtrl(fullbody,selectedLinea,preSelect,&selectedLinea->pBottom);
                }else if (event.key.code == sf::Keyboard::M){
                    //spine
                    selectedLinea = &fullbody[4];
                    selectCtrl(fullbody,selectedLinea,preSelect,&selectedLinea->pBottom);
                }else if (event.key.code == sf::Keyboard::Tab){
                    movEjeX = true ;
                }else if (event.key.code == sf::Keyboard::G){
                    movEjeX = false;
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
                    // 1. Regresar el marcador al inicio del timeline
                    current_market.setPosition(10.f, 10.f);
                    isPlaying = false;

                    // 2. Actualizar la elipse al frame de inicio (10.f) si existe
                    for (const auto& kf : elipceData){
                        if (kf.frame == 10.f){
                            elipce.setPosition(kf.x, kf.y);
                            break;
                        }
                    }

                    // 3. Actualizar el cuerpo (fullbody) al keyframe de inicio (10.f) si existe
                    auto bodyIt = std::find_if(
                        keyPartData.begin(),
                        keyPartData.end(),
                        [](const bodyKeyframe& k){
                            return k.frame == 10.f;
                        }
                    );
                    if (bodyIt != keyPartData.end()){
                        applyBodyKeyframe(*bodyIt, fullbody);
                    }

                }else{
                    current_market.setPosition(pos.x , 10.f);
                    for (const auto& kf : elipceData){
                        if (kf.frame == pos.x){
                            elipce.setPosition(kf.x, kf.y);
                            break;
                        }
                    }
                    auto bodyIt = std::find_if(
                        keyPartData.begin(),
                        keyPartData.end(),
                        [pos](const bodyKeyframe& k){
                            return k.frame == pos.x;
                        }
                    );
                    if (bodyIt != keyPartData.end()){
                        applyBodyKeyframe(*bodyIt , fullbody);
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
        for (const auto& kf : listaKeyBody){
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
            for (const auto& onionKey : onionBodyData){
                std::vector<linealBezier> tempBody = fullbody;
                applyBodyKeyframe(onionKey , tempBody);
                for(auto& bodyPart : tempBody){
                    bodyPart.drawMode = DrawMode::Onion;
                    bodyPart.draw(window);
                    bodyPart.drawMode = DrawMode::Normal;
                }
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
                // 1. Actualizar la posición de la elipse según el frame actual del render
                for (const auto& kf : elipceData)
                {
                    if (kf.frame == framePos)
                    {
                        elipce.setPosition(kf.x, kf.y);
                        break;
                    }
                }

                // 2. Buscar y aplicar la pose del cuerpo (keyPartData) correspondiente a este frame
                auto bodyIt = std::find_if(
                    keyPartData.begin(),
                    keyPartData.end(),
                    [framePos](const bodyKeyframe& k){
                        return k.frame == framePos;
                    }
                );
                if (bodyIt != keyPartData.end())
                {
                    applyBodyKeyframe(*bodyIt, fullbody);
                }

                // 3. Dibujar la escena, la elipse y el cuerpo actualizado
                render.clear(sf::Color::White);
                render.draw(drawScena);
                render.draw(elipce);

                for (auto& bodyPart : fullbody){
                    bodyPart.draw(render);
                }

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
        // PIN: Dibuja fullbody
        for (auto& bodyPart : fullbody){
            bodyPart.draw(window);
        }
        window.display();
                
    }
        return 0;
}
