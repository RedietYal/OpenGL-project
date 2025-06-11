#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <iostream>
#include <vector>
#include <random>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <map>
#include <array>

using namespace std;

const int GRID_WIDTH = 15;
const int GRID_HEIGHT = 20;
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 800;
const float BLOCK_SIZE = 30.0f;
const float GRID_OFFSET_X = 50.0f;
const float GRID_OFFSET_Y = 50.0f;
const float BORDER_WIDTH = 3.0f;

const int TETROMINO_SHAPES[7][4][4] = {
    {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,1,0,0}, {1,1,1,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0}},
    {{0,0,0,0}, {1,0,0,0}, {1,1,1,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,0,1,0}, {1,1,1,0}, {0,0,0,0}}
};

const float TETROMINO_COLORS[7][3] = {
    {0.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 0.0f},
    {0.5f, 0.0f, 0.5f},
    {0.0f, 1.0f, 0.0f},
    {1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f},
    {1.0f, 0.5f, 0.0f}
};

const int SCORE_VALUES[4] = {40, 100, 300, 1200};

struct Tetromino {
    int shape[4][4];
    float color[3];
    int x, y;
    int type;
    
    Tetromino() {
        memset(shape, 0, sizeof(shape));
        memset(color, 0, sizeof(color));
        x = y = type = 0;
    }
    
    void setType(int newType) {
        type = newType;
        memcpy(shape, TETROMINO_SHAPES[newType], sizeof(shape));
        memcpy(color, TETROMINO_COLORS[newType], sizeof(color));
    }
};

class TetrisGame {
private:
    int grid[GRID_HEIGHT][GRID_WIDTH];
    float gridColors[GRID_HEIGHT][GRID_WIDTH][3];
    Tetromino currentPiece;
    Tetromino nextPiece;
    double lastFallTime;
    double fallSpeed;
    double baseFallSpeed;
    bool gameOver;
    bool gamePaused;
    bool showHelp;
    int score;
    int level;
    int linesCleared;
    
    mt19937 rng;
    uniform_int_distribution<int> shapeDist;
    
    GLuint VAO, VBO;
    GLuint shaderProgram;
    
    bool restartButtonHovered;
    bool helpButtonHovered;
    bool closeHelpButtonHovered;
    float restartButtonX, restartButtonY, restartButtonW, restartButtonH;
    float helpButtonX, helpButtonY, helpButtonW, helpButtonH;
    float closeHelpButtonX, closeHelpButtonY, closeHelpButtonW, closeHelpButtonH;
    
    bool keyStates[GLFW_KEY_LAST] = {false};
    bool prevKeyStates[GLFW_KEY_LAST] = {false};
    bool mousePressed = false;
    bool prevMousePressed = false;
    double mouseX = 0, mouseY = 0;
    
    map<char, array<array<int, 5>, 7>> fontData;
    
public:
    TetrisGame() : rng(random_device{}()), shapeDist(0, 6) {
        memset(grid, 0, sizeof(grid));
        memset(gridColors, 0, sizeof(gridColors));
        
        lastFallTime = 0.0;
        baseFallSpeed = 1.0;
        fallSpeed = baseFallSpeed;
        gameOver = false;
        gamePaused = false;
        showHelp = false;
        score = 0;
        level = 1;
        linesCleared = 0;
        
        restartButtonHovered = false;
        helpButtonHovered = false;
        closeHelpButtonHovered = false;
        restartButtonX = WINDOW_WIDTH - 180;
        restartButtonY = 400;
        restartButtonW = 120;
        restartButtonH = 40;
        
        helpButtonX = WINDOW_WIDTH - 180;
        helpButtonY = 500;
        helpButtonW = 120;
        helpButtonH = 40;

        closeHelpButtonX = WINDOW_WIDTH / 2 - 60;
        closeHelpButtonY = WINDOW_HEIGHT / 2 + 150;
        closeHelpButtonW = 120;
        closeHelpButtonH = 40;
        
        initializeFont();
        
        nextPiece.setType(shapeDist(rng));
        spawnNewPiece();
        setupOpenGL();
    }
    
    void initializeFont() {
        fontData['0'] = {{
            {1,1,1,1,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,1,1,1,1}
        }};
        
        fontData['1'] = {{
            {0,0,1,0,0},
            {0,1,1,0,0},
            {0,0,1,0,0},
            {0,0,1,0,0},
            {0,0,1,0,0},
            {0,0,1,0,0},
            {0,1,1,1,0}
        }};
        
        fontData['2'] = {{
            {1,1,1,1,1},
            {0,0,0,0,1},
            {0,0,0,0,1},
            {1,1,1,1,1},
            {1,0,0,0,0},
            {1,0,0,0,0},
            {1,1,1,1,1}
        }};
        
        fontData['3'] = {{
            {1,1,1,1,1},
            {0,0,0,0,1},
            {0,0,0,0,1},
            {1,1,1,1,1},
            {0,0,0,0,1},
            {0,0,0,0,1},
            {1,1,1,1,1}
        }};
        
        fontData['4'] = {{
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,1,1,1,1},
            {0,0,0,0,1},
            {0,0,0,0,1},
            {0,0,0,0,1}
        }};
        
        fontData['5'] = {{
            {1,1,1,1,1},
            {1,0,0,0,0},
            {1,0,0,0,0},
            {1,1,1,1,1},
            {0,0,0,0,1},
            {0,0,0,0,1},
            {1,1,1,1,1}
        }};
        
        fontData['6'] = {{
            {1,1,1,1,1},
            {1,0,0,0,0},
            {1,0,0,0,0},
            {1,1,1,1,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,1,1,1,1}
        }};
        
        fontData['7'] = {{
            {1,1,1,1,1},
            {0,0,0,0,1},
            {0,0,0,0,1},
            {0,0,0,1,0},
            {0,0,1,0,0},
            {0,1,0,0,0},
            {1,0,0,0,0}
        }};
        
        fontData['8'] = {{
            {1,1,1,1,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,1,1,1,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,1,1,1,1}
        }};
        
        fontData['9'] = {{
            {1,1,1,1,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,1,1,1,1},
            {0,0,0,0,1},
            {0,0,0,0,1},
            {1,1,1,1,1}
        }};
        
        fontData['S'] = {{
            {1,1,1,1,1},
            {1,0,0,0,0},
            {1,0,0,0,0},
            {1,1,1,1,1},
            {0,0,0,0,1},
            {0,0,0,0,1},
            {1,1,1,1,1}
        }};
        
        fontData['C'] = {{
            {1,1,1,1,1},
            {1,0,0,0,0},
            {1,0,0,0,0},
            {1,0,0,0,0},
            {1,0,0,0,0},
            {1,0,0,0,0},
            {1,1,1,1,1}
        }};
        
        fontData['O'] = {{
            {1,1,1,1,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,1,1,1,1}
        }};
        
        fontData['R'] = {{
            {1,1,1,1,0},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,1,1,1,0},
            {1,0,1,0,0},
            {1,0,0,1,0},
            {1,0,0,0,1}
        }};
        
        fontData['E'] = {{
            {1,1,1,1,1},
            {1,0,0,0,0},
            {1,0,0,0,0},
            {1,1,1,1,0},
            {1,0,0,0,0},
            {1,0,0,0,0},
            {1,1,1,1,1}
        }};
        
        fontData['L'] = {{
            {1,0,0,0,0},
            {1,0,0,0,0},
            {1,0,0,0,0},
            {1,0,0,0,0},
            {1,0,0,0,0},
            {1,0,0,0,0},
            {1,1,1,1,1}
        }};
        
        fontData['V'] = {{
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {0,1,0,1,0},
            {0,0,1,0,0}
        }};
        
        fontData['N'] = {{
            {1,0,0,0,1},
            {1,1,0,0,1},
            {1,0,1,0,1},
            {1,0,0,1,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1}
        }};
        
        fontData['X'] = {{
            {1,0,0,0,1},
            {0,1,0,1,0},
            {0,0,1,0,0},
            {0,0,1,0,0},
            {0,0,1,0,0},
            {0,1,0,1,0},
            {1,0,0,0,1}
        }};
        
        fontData['T'] = {{
            {1,1,1,1,1},
            {0,0,1,0,0},
            {0,0,1,0,0},
            {0,0,1,0,0},
            {0,0,1,0,0},
            {0,0,1,0,0},
            {0,0,1,0,0}
        }};
        
        fontData[':'] = {{
            {0,0,0,0,0},
            {0,0,1,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0},
            {0,0,1,0,0},
            {0,0,0,0,0}
        }};
        
        fontData[' '] = {{
            {0,0,0,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0}
        }};
        
        fontData['P'] = {{
            {1,1,1,1,0},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,1,1,1,0},
            {1,0,0,0,0},
            {1,0,0,0,0},
            {1,0,0,0,0}
        }};
        
        fontData['A'] = {{
            {0,1,1,1,0},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,1,1,1,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1}
        }};
        
        fontData['U'] = {{
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,1,1,1,1}
        }};
        
        fontData['G'] = {{
            {1,1,1,1,1},
            {1,0,0,0,0},
            {1,0,0,0,0},
            {1,0,1,1,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,1,1,1,1}
        }};
        
        fontData['M'] = {{
            {1,0,0,0,1},
            {1,1,0,1,1},
            {1,0,1,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1}
        }};
        
        fontData['I'] = {{
            {1,1,1,1,1},
            {0,0,1,0,0},
            {0,0,1,0,0},
            {0,0,1,0,0},
            {0,0,1,0,0},
            {0,0,1,0,0},
            {1,1,1,1,1}
        }};
        
        fontData['H'] = {{
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,1,1,1,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1}
        }};
        
        fontData['W'] = {{
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,1,0,1},
            {1,0,1,0,1},
            {1,1,0,1,1},
            {1,0,0,0,1}
        }};
        
        fontData['D'] = {{
            {1,1,1,1,0},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,1,1,1,0}
        }};
        
        fontData['F'] = {{
            {1,1,1,1,1},
            {1,0,0,0,0},
            {1,0,0,0,0},
            {1,1,1,1,0},
            {1,0,0,0,0},
            {1,0,0,0,0},
            {1,0,0,0,0}
        }};
        
        fontData['Y'] = {{
            {1,0,0,0,1},
            {1,0,0,0,1},
            {0,1,0,1,0},
            {0,0,1,0,0},
            {0,0,1,0,0},
            {0,0,1,0,0},
            {0,0,1,0,0}
        }};
        
        fontData['K'] = {{
            {1,0,0,0,1},
            {1,0,0,1,0},
            {1,0,1,0,0},
            {1,1,0,0,0},
            {1,0,1,0,0},
            {1,0,0,1,0},
            {1,0,0,0,1}
        }};
        
        fontData['-'] = {{
            {0,0,0,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0},
            {1,1,1,1,1},
            {0,0,0,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0}
        }};
        
        fontData['/'] = {{
            {0,0,0,0,1},
            {0,0,0,1,0},
            {0,0,1,0,0},
            {0,1,0,0,0},
            {1,0,0,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0}
        }};
    }
    
    void drawCharacter(char c, float x, float y, const float color[3], float pixelSize = 3.0f) {
        if (fontData.find(c) == fontData.end()) return;
        
        auto& charData = fontData[c];
        for (int row = 0; row < 7; row++) {
            for (int col = 0; col < 5; col++) {
                if (charData[row][col]) {
                    drawPixel(x + col * pixelSize, y + row * pixelSize, pixelSize, color);
                }
            }
        }
    }
    
    void drawText(const string& text, float x, float y, const float color[3], float pixelSize = 3.0f) {
        float currentX = x;
        for (char c : text) {
            drawCharacter(c, currentX, y, color, pixelSize);
            currentX += 6 * pixelSize;
        }
    }
    
    void drawPixel(float x, float y, float size, const float color[3]) {
        glUseProgram(shaderProgram);
        
        GLint translationLoc = glGetUniformLocation(shaderProgram, "uTranslation");
        GLint scaleLoc = glGetUniformLocation(shaderProgram, "uScale");
        GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");
        GLint brightnessLoc = glGetUniformLocation(shaderProgram, "uBrightness");
        
        glUniform2f(translationLoc, x, y);
        glUniform2f(scaleLoc, size, size);
        glUniform3f(colorLoc, color[0], color[1], color[2]);
        glUniform1f(brightnessLoc, 1.0f);
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    void drawButton(float x, float y, float w, float h, const string& text, bool hovered, bool pressed = false) {
        float bgColor[3];
        if (text == "RESTART") {
            if (pressed) {
                bgColor[0] = 0.1f; bgColor[1] = 0.4f; bgColor[2] = 0.1f;
            } else if (hovered) {
                bgColor[0] = 0.2f; bgColor[1] = 0.8f; bgColor[2] = 0.2f;
            } else {
                bgColor[0] = 0.1f; bgColor[1] = 0.6f; bgColor[2] = 0.1f;
            }
        } else if (text == "HELP") {
            if (pressed) {
                bgColor[0] = 0.6f; bgColor[1] = 0.6f; bgColor[2] = 0.0f;
            } else if (hovered) {
                bgColor[0] = 1.0f; bgColor[1] = 1.0f; bgColor[2] = 0.3f;
            } else {
                bgColor[0] = 0.8f; bgColor[1] = 0.8f; bgColor[2] = 0.0f;
            }
        } else {
            if (pressed) {
                bgColor[0] = 0.2f; bgColor[1] = 0.6f; bgColor[2] = 0.2f;
            } else if (hovered) {
                bgColor[0] = 0.3f; bgColor[1] = 0.7f; bgColor[2] = 0.3f;
            } else {
                bgColor[0] = 0.4f; bgColor[1] = 0.4f; bgColor[2] = 0.4f;
            }
        }
        
        glUseProgram(shaderProgram);
        GLint translationLoc = glGetUniformLocation(shaderProgram, "uTranslation");
        GLint scaleLoc = glGetUniformLocation(shaderProgram, "uScale");
        GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");
        GLint brightnessLoc = glGetUniformLocation(shaderProgram, "uBrightness");
        
        glUniform2f(translationLoc, x, y);
        glUniform2f(scaleLoc, w, h);
        glUniform3f(colorLoc, bgColor[0], bgColor[1], bgColor[2]);
        glUniform1f(brightnessLoc, 1.0f);
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        float textColor[3] = {1.0f, 1.0f, 1.0f};
        float textX = x + (w - text.length() * 6 * 2.5f) / 2;
        float textY = y + (h - 7 * 2.5f) / 2;
        drawText(text, textX, textY, textColor, 2.5f);
    }
    
    void drawBorder() {
        float borderColor[3] = {0.8f, 0.8f, 0.8f};
        
        glUseProgram(shaderProgram);
        GLint translationLoc = glGetUniformLocation(shaderProgram, "uTranslation");
        GLint scaleLoc = glGetUniformLocation(shaderProgram, "uScale");
        GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");
        GLint brightnessLoc = glGetUniformLocation(shaderProgram, "uBrightness");
        
        glUniform3f(colorLoc, borderColor[0], borderColor[1], borderColor[2]);
        glUniform1f(brightnessLoc, 1.0f);
        
        float gridAreaX = GRID_OFFSET_X;
        float gridAreaY = GRID_OFFSET_Y;
        float gridAreaW = GRID_WIDTH * BLOCK_SIZE;
        float gridAreaH = GRID_HEIGHT * BLOCK_SIZE;
        
        glUniform2f(translationLoc, gridAreaX - BORDER_WIDTH, gridAreaY - BORDER_WIDTH);
        glUniform2f(scaleLoc, gridAreaW + 2 * BORDER_WIDTH, BORDER_WIDTH);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glUniform2f(translationLoc, gridAreaX - BORDER_WIDTH, gridAreaY + gridAreaH);
        glUniform2f(scaleLoc, gridAreaW + 2 * BORDER_WIDTH, BORDER_WIDTH);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glUniform2f(translationLoc, gridAreaX - BORDER_WIDTH, gridAreaY);
        glUniform2f(scaleLoc, BORDER_WIDTH, gridAreaH);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glUniform2f(translationLoc, gridAreaX + gridAreaW, gridAreaY);
        glUniform2f(scaleLoc, BORDER_WIDTH, gridAreaH);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    void drawHelpOverlay() {
        if (!showHelp) return;
        
        float overlayColor[3] = {0.0f, 0.0f, 0.0f};
        glUseProgram(shaderProgram);
        GLint translationLoc = glGetUniformLocation(shaderProgram, "uTranslation");
        GLint scaleLoc = glGetUniformLocation(shaderProgram, "uScale");
        GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");
        GLint brightnessLoc = glGetUniformLocation(shaderProgram, "uBrightness");
        
        glUniform2f(translationLoc, 0, 0);
        glUniform2f(scaleLoc, WINDOW_WIDTH, WINDOW_HEIGHT);
        glUniform3f(colorLoc, overlayColor[0], overlayColor[1], overlayColor[2]);
        glUniform1f(brightnessLoc, 0.7f);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        float panelColor[3] = {0.2f, 0.2f, 0.3f};
        float panelX = WINDOW_WIDTH / 2 - 200;
        float panelY = WINDOW_HEIGHT / 2 - 200;
        float panelW = 400;
        float panelH = 400;
        
        glUniform2f(translationLoc, panelX, panelY);
        glUniform2f(scaleLoc, panelW, panelH);
        glUniform3f(colorLoc, panelColor[0], panelColor[1], panelColor[2]);
        glUniform1f(brightnessLoc, 1.0f);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        float textColor[3] = {1.0f, 1.0f, 1.0f};
        float titleColor[3] = {0.0f, 1.0f, 1.0f};
        float textX = panelX + 20;
        float textY = panelY + 20;
        
        drawText("HOW TO PLAY TETRIS", textX, textY, titleColor, 2.5f);
        
        drawText("CONTROLS:", textX, textY + 40, textColor, 2.0f);
        drawText("A/D OR LEFT/RIGHT - MOVE", textX, textY + 60, textColor, 1.8f);
        drawText("W OR UP - ROTATE PIECE", textX, textY + 80, textColor, 1.8f);
        drawText("S OR DOWN - SOFT DROP", textX, textY + 100, textColor, 1.8f);
        drawText("SPACE - HARD DROP", textX, textY + 120, textColor, 1.8f);
        drawText("P - PAUSE/RESUME", textX, textY + 140, textColor, 1.8f);
        drawText("R - RESTART GAME", textX, textY + 160, textColor, 1.8f);
        
        drawText("OBJECTIVE:", textX, textY + 190, textColor, 2.0f);
        drawText("FILL COMPLETE ROWS TO", textX, textY + 210, textColor, 1.8f);
        drawText("CLEAR THEM AND SCORE", textX, textY + 230, textColor, 1.8f);
        drawText("POINTS", textX, textY + 250, textColor, 1.8f);
        
        drawText("SCORING:", textX, textY + 280, textColor, 2.0f);
        drawText("1 LINE = 40 X LEVEL", textX, textY + 300, textColor, 1.8f);
        drawText("2 LINES = 100 X LEVEL", textX, textY + 320, textColor, 1.8f);
        drawText("3 LINES = 300 X LEVEL", textX, textY + 340, textColor, 1.8f);
        drawText("4 LINES = 1200 X LEVEL", textX, textY + 360, textColor, 1.8f);
        
        float closeColor[3] = {1.0f, 1.0f, 0.0f};
        drawText("CLICK CLOSE BUTTON TO RETURN", textX, textY + 380, closeColor, 1.5f);

        drawButton(closeHelpButtonX, closeHelpButtonY, closeHelpButtonW, closeHelpButtonH, 
                  "CLOSE", closeHelpButtonHovered);
    }
    
    void setupOpenGL() {
        const char* vertexShaderSource = R"(
            #version 330 core
            layout (location = 0) in vec2 aPos;
            uniform vec2 uTranslation;
            uniform vec2 uScale;
            void main() {
                vec2 pos = (aPos * uScale) + uTranslation;
                gl_Position = vec4(pos.x * 2.0 / 1000.0 - 1.0, 1.0 - pos.y * 2.0 / 800.0, 0.0, 1.0);
            }
        )";
        
        const char* fragmentShaderSource = R"(
            #version 330 core
            out vec4 FragColor;
            uniform vec3 uColor;
            uniform float uBrightness;
            void main() {
                FragColor = vec4(uColor * uBrightness, 1.0);
            }
        )";
        
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            cout << "Vertex shader compilation failed: " << infoLog << endl;
        }
        
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            cout << "Fragment shader compilation failed: " << infoLog << endl;
        }
        
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            cout << "Shader program linking failed: " << infoLog << endl;
        }
        
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        
        float vertices[] = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
        };
        
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    
    void spawnNewPiece() {
        currentPiece = nextPiece;
        currentPiece.x = GRID_WIDTH / 2 - 2;
        currentPiece.y = 0;
        
        nextPiece.setType(shapeDist(rng));
        
        if (checkCollision(currentPiece, 0, 0)) {
            gameOver = true;
        }
    }
    
    bool checkCollision(const Tetromino& piece, int dx, int dy) {
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                if (piece.shape[y][x]) {
                    int newX = piece.x + x + dx;
                    int newY = piece.y + y + dy;
                    
                    if (newX < 0 || newX >= GRID_WIDTH || newY >= GRID_HEIGHT) {
                        return true;
                    }
                    
                    if (newY >= 0 && grid[newY][newX]) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
    
    void placePiece() {
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                if (currentPiece.shape[y][x]) {
                    int gridX = currentPiece.x + x;
                    int gridY = currentPiece.y + y;
                    
                    if (gridY >= 0) {
                        grid[gridY][gridX] = 1;
                        memcpy(gridColors[gridY][gridX], currentPiece.color, sizeof(currentPiece.color));
                    }
                }
            }
        }
        
        int clearedLines = clearLines();
        updateScore(clearedLines);
        spawnNewPiece();
    }
    
    int clearLines() {
        int clearedCount = 0;
        
        for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
            bool fullLine = true;
            for (int x = 0; x < GRID_WIDTH; x++) {
                if (!grid[y][x]) {
                    fullLine = false;
                    break;
                }
            }
            
            if (fullLine) {
                clearedCount++;
                
                for (int moveY = y; moveY > 0; moveY--) {
                    for (int x = 0; x < GRID_WIDTH; x++) {
                        grid[moveY][x] = grid[moveY - 1][x];
                        memcpy(gridColors[moveY][x], gridColors[moveY - 1][x], sizeof(gridColors[moveY][x]));
                    }
                }
                
                for (int x = 0; x < GRID_WIDTH; x++) {
                    grid[0][x] = 0;
                    memset(gridColors[0][x], 0, sizeof(gridColors[0][x]));
                }
                
                y++;
            }
        }
        
        return clearedCount;
    }
    
    void updateScore(int clearedLines) {
        if (clearedLines > 0) {
            linesCleared += clearedLines;
            score += SCORE_VALUES[clearedLines - 1] * level;
            
            int newLevel = (linesCleared / 10) + 1;
            if (newLevel > level) {
                level = newLevel;
                fallSpeed = baseFallSpeed / (1.0 + (level - 1) * 0.1);
            }
        }
    }
    
    void rotatePiece() {
        Tetromino rotated = currentPiece;
        
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                rotated.shape[x][3-y] = currentPiece.shape[y][x];
            }
        }
        
        if (!checkCollision(rotated, 0, 0)) {
            currentPiece = rotated;
            return;
        }
        
        int kicks[][2] = {{-1, 0}, {1, 0}, {0, -1}, {-1, -1}, {1, -1}};
        for (int i = 0; i < 5; i++) {
            if (!checkCollision(rotated, kicks[i][0], kicks[i][1])) {
                currentPiece = rotated;
                currentPiece.x += kicks[i][0];
                currentPiece.y += kicks[i][1];
                return;
            }
        }
    }
    
    void update(double currentTime) {
        if (gameOver || gamePaused || showHelp) return;
        
        if (currentTime - lastFallTime >= fallSpeed) {
            if (!checkCollision(currentPiece, 0, 1)) {
                currentPiece.y++;
            } else {
                placePiece();
            }
            lastFallTime = currentTime;
        }
    }
    
    bool isKeyPressed(int key) {
        return keyStates[key] && !prevKeyStates[key];
    }
    
    void updateInput(GLFWwindow* window) {
        memcpy(prevKeyStates, keyStates, sizeof(keyStates));
        prevMousePressed = mousePressed;
        
        for (int i = 0; i < GLFW_KEY_LAST; i++) {
            keyStates[i] = glfwGetKey(window, i) == GLFW_PRESS;
        }
        
        mousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        
        restartButtonHovered = (mouseX >= restartButtonX && mouseX <= restartButtonX + restartButtonW &&
                               mouseY >= restartButtonY && mouseY <= restartButtonY + restartButtonH);

        helpButtonHovered = (mouseX >= helpButtonX && mouseX <= helpButtonX + helpButtonW &&
                            mouseY >= helpButtonY && mouseY <= helpButtonY + helpButtonH);

        closeHelpButtonHovered = showHelp && (mouseX >= closeHelpButtonX && mouseX <= closeHelpButtonX + closeHelpButtonW &&
                        mouseY >= closeHelpButtonY && mouseY <= closeHelpButtonY + closeHelpButtonH);
    }
    
    bool isMouseClicked() {
        return mousePressed && !prevMousePressed;
    }
    
    void handleInput(GLFWwindow* window) {
        updateInput(window);
        
        if (isMouseClicked()) {
            if (restartButtonHovered) {
                restartGame();
                return;
            }
            if (helpButtonHovered) {
                showHelp = !showHelp;
                return;
            }
            if (closeHelpButtonHovered) {
                showHelp = false;
                return;
            }
        }
        
        if (showHelp) return;
        
        if (isKeyPressed(GLFW_KEY_P)) {
            gamePaused = !gamePaused;
        }
        
        if (isKeyPressed(GLFW_KEY_R)) {
            restartGame();
        }
        
        if (gameOver || gamePaused) return;
        
        if (isKeyPressed(GLFW_KEY_LEFT) || isKeyPressed(GLFW_KEY_A)) {
            if (!checkCollision(currentPiece, -1, 0)) {
                currentPiece.x--;
            }
        }
        
        if (isKeyPressed(GLFW_KEY_RIGHT) || isKeyPressed(GLFW_KEY_D)) {
            if (!checkCollision(currentPiece, 1, 0)) {
                currentPiece.x++;
            }
        }
        
        if (isKeyPressed(GLFW_KEY_DOWN) || isKeyPressed(GLFW_KEY_S)) {
            if (!checkCollision(currentPiece, 0, 1)) {
                currentPiece.y++;
            }
        }
        
        if (isKeyPressed(GLFW_KEY_SPACE)) {
            while (!checkCollision(currentPiece, 0, 1)) {
                currentPiece.y++;
            }
        }
        
        if (isKeyPressed(GLFW_KEY_UP) || isKeyPressed(GLFW_KEY_W)) {
            rotatePiece();
        }
    }
    
    void restartGame() {
        memset(grid, 0, sizeof(grid));
        memset(gridColors, 0, sizeof(gridColors));
        gameOver = false;
        gamePaused = false;
        showHelp = false;
        score = 0;
        level = 1;
        linesCleared = 0;
        fallSpeed = baseFallSpeed;
        nextPiece.setType(shapeDist(rng));
        spawnNewPiece();
    }
    
    void drawBlock(float x, float y, const float color[3], float brightness = 1.0f) {
        glUseProgram(shaderProgram);
        
        GLint translationLoc = glGetUniformLocation(shaderProgram, "uTranslation");
        GLint scaleLoc = glGetUniformLocation(shaderProgram, "uScale");
        GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");
        GLint brightnessLoc = glGetUniformLocation(shaderProgram, "uBrightness");
        
        glUniform2f(translationLoc, x * BLOCK_SIZE + GRID_OFFSET_X, y * BLOCK_SIZE + GRID_OFFSET_Y);
        glUniform2f(scaleLoc, BLOCK_SIZE - 1, BLOCK_SIZE - 1);
        glUniform3f(colorLoc, color[0], color[1], color[2]);
        glUniform1f(brightnessLoc, brightness);
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    void render() {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        
        drawBorder();
        
        float gridColor[3] = {0.2f, 0.2f, 0.3f};
        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {
                drawBlock(x, y, gridColor, 0.3f);
            }
        }
        
        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {
                if (grid[y][x]) {
                    drawBlock(x, y, gridColors[y][x]);
                }
            }
        }
        
        if (!gameOver && !gamePaused && !showHelp) {
            for (int y = 0; y < 4; y++) {
                for (int x = 0; x < 4; x++) {
                    if (currentPiece.shape[y][x]) {
                        drawBlock(currentPiece.x + x, currentPiece.y + y, currentPiece.color);
                    }
                }
            }
        }
        
        float panelColor[3] = {0.15f, 0.15f, 0.2f};
        glUseProgram(shaderProgram);
        GLint translationLoc = glGetUniformLocation(shaderProgram, "uTranslation");
        GLint scaleLoc = glGetUniformLocation(shaderProgram, "uScale");
        GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");
        GLint brightnessLoc = glGetUniformLocation(shaderProgram, "uBrightness");
        
        float panelX = WINDOW_WIDTH - 220;
        float panelY = GRID_OFFSET_Y;
        float panelW = 200;
        float panelH = GRID_HEIGHT * BLOCK_SIZE;
        
        glUniform2f(translationLoc, panelX, panelY);
        glUniform2f(scaleLoc, panelW, panelH);
        glUniform3f(colorLoc, panelColor[0], panelColor[1], panelColor[2]);
        glUniform1f(brightnessLoc, 1.0f);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        float textColor[3] = {1.0f, 1.0f, 1.0f};
        float uiX = panelX + 10;
        float uiY = panelY + 20;
        
        drawText("SCORE:", uiX, uiY, textColor, 2.5f);
        drawText(to_string(score), uiX, uiY + 25, textColor, 2.5f);

        drawText("LEVEL:", uiX, uiY + 65, textColor, 2.5f);
        drawText(to_string(level), uiX, uiY + 90, textColor, 2.5f);

        drawText("LINES:", uiX, uiY + 130, textColor, 2.5f);
        drawText(to_string(linesCleared), uiX, uiY + 155, textColor, 2.5f);

        drawText("NEXT:", uiX, uiY + 195, textColor, 2.5f);
        float previewX = (panelX + 20 - GRID_OFFSET_X) / BLOCK_SIZE;
        float previewY = (uiY + 220 - GRID_OFFSET_Y) / BLOCK_SIZE;
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                if (nextPiece.shape[y][x]) {
                    drawBlock(previewX + x, previewY + y, nextPiece.color, 0.8f);
                }
            }
        }
        
        restartButtonX = panelX + 40;
        restartButtonY = 370;
        drawButton(restartButtonX, restartButtonY, restartButtonW, restartButtonH, 
                  "RESTART", restartButtonHovered);

        helpButtonX = panelX + 40;
        helpButtonY = 470;
        drawButton(helpButtonX, helpButtonY, helpButtonW, helpButtonH, 
                  "HELP", helpButtonHovered);
        
        if (gamePaused) {
            float pauseColor[3] = {1.0f, 1.0f, 0.0f};
            drawText("PAUSED", uiX, uiY + 480, pauseColor, 3.0f);
        }
        
        if (gameOver) {
            float gameOverColor[3] = {1.0f, 0.0f, 0.0f};
            drawText("GAME", uiX, uiY + 480, gameOverColor, 3.0f);
            drawText("OVER", uiX, uiY + 510, gameOverColor, 3.0f);
        }
        
        drawHelpOverlay();
    }
    
    bool isGameOver() const {
        return gameOver;
    }
    
    bool isPaused() const {
        return gamePaused;
    }
    
    int getScore() const {
        return score;
    }
    
    int getLevel() const {
        return level;
    }
    
    int getLines() const {
        return linesCleared;
    }
};

int main() {
    if (!glfwInit()) {
        cerr << "Failed to initialize GLFW" << endl;
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    
    int xpos = (mode->width - WINDOW_WIDTH) / 2;
    int ypos = (mode->height - WINDOW_HEIGHT) / 2;
    
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tetris - OpenGL", NULL, NULL);
    if (!window) {
        cerr << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    
    glfwSetWindowPos(window, xpos, ypos);
    
    glfwMakeContextCurrent(window);
    glfwFocusWindow(window);
    glfwSwapInterval(1);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cerr << "Failed to initialize GLAD" << endl;
        return -1;
    }
    
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    TetrisGame game;
    
    cout << "Tetris Game Started!" << endl;
    cout << "Use WASD or Arrow Keys to play" << endl;
    cout << "Click RESTART button or press R to restart" << endl;
    cout << "Click HELP button for game instructions" << endl;
    
    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        
        game.handleInput(window);
        game.update(currentTime);
        game.render();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }
    }
    
    glfwTerminate();
    return 0;
}
