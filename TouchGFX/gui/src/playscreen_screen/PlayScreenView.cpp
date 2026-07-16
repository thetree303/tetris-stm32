#include <gui/playscreen_screen/PlayScreenView.hpp>
#include "stm32f4xx_hal.h"
#include "audio.h"
#include <images/BitmapDatabase.hpp>
#include <touchgfx/Unicode.hpp>
#include <touchgfx/Color.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <stdio.h>
#include <stdlib.h>


// DỮ LIỆU HÌNH DẠNG 7 KHỐI
// Giá trị mỗi ô (mỗi phần tử mảng): 0 = ô trống, 1-7 = màu sắc tương ứng với các ô (tương ứng với loại khối)
static const int TETROMINOES[7][4][4] = {
    // Khối I (type 1) - cyan
    {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
    // Khối O (type 2) - vàng
    {{0,0,0,0}, {0,2,2,0}, {0,2,2,0}, {0,0,0,0}},
    // Khối T (type 3) - tím
    {{0,0,0,0}, {0,3,3,3}, {0,0,3,0}, {0,0,0,0}},
    // Khối L (type 4) - cam
    {{0,0,0,0}, {0,4,4,4}, {0,4,0,0}, {0,0,0,0}},
    // Khối J (type 5) - xanh dương
    {{0,0,0,0}, {0,5,5,5}, {0,0,0,5}, {0,0,0,0}},
    // Khối S (type 6) - xanh lá
    {{0,0,0,0}, {0,0,6,6}, {0,6,6,0}, {0,0,0,0}},
    // Khối Z (type 7) - đỏ
    {{0,0,0,0}, {0,7,7,0}, {0,0,7,7}, {0,0,0,0}}
};

// Ánh xạ type => BitmapID (index 0 không dùng)
static const uint16_t BLOCK_BITMAPS[8] = {
    0,
    BITMAP_BLOCK1_ID,   // 1: I
    BITMAP_BLOCK2_ID,   // 2: O
    BITMAP_BLOCK3_ID,   // 3: T
    BITMAP_BLOCK4_ID,   // 4: L
    BITMAP_BLOCK5_ID,   // 5: J
    BITMAP_BLOCK6_ID,   // 6: S
    BITMAP_BLOCK7_ID    // 7: Z
};

// Ánh xạ cho Next Block (4x4), tương tự block bitmap
static const uint16_t MINI_BLOCK_BITMAPS[8] = {
    0, 
    BITMAP_BLOCK1_MINI_ID,   
    BITMAP_BLOCK2_MINI_ID,   
    BITMAP_BLOCK3_MINI_ID,   
    BITMAP_BLOCK4_MINI_ID,   
    BITMAP_BLOCK5_MINI_ID,   
    BITMAP_BLOCK6_MINI_ID,   
    BITMAP_BLOCK7_MINI_ID    
};

// Lấy ngẫu nhiên 1 khối (sử dụng thuật toán 7-Bag)
static int getTetrisBagPiece()
{
    static int bag[7] = {0, 1, 2, 3, 4, 5, 6};  // Mảng chứa 7 loại khối (từ 0 đến 6)

    static int currentIndex = 7; // Đặt bằng 7 để ép xáo trộn ngay lần gọi đầu tiên
    static bool isSeeded = false;

    // Khởi tạo seed đúng 1 lần
    if (!isSeeded)
    {
        srand(HAL_GetTick());
        isSeeded = true;
    }

    // Nếu đã bốc hết 7 khối trong túi => Trộn túi mới
    if (currentIndex >= 7)
    {
        // Thuật toán xáo trộn Fisher-Yates
        for (int i = 6; i > 0; --i)
        {
            // Chọn ngẫu nhiên một chỉ số j từ 0 đến i
            int j = rand() % (i + 1); 
            
            // Đổi chỗ bag[i] và bag[j]
            int temp = bag[i];
            bag[i] = bag[j];
            bag[j] = temp;
        }

        currentIndex = 0; // Trở về đầu túi
    }

    // Rút khối ở vị trí currentIndex và tăng index lên
    return bag[currentIndex++];
}

PlayScreenView::PlayScreenView()
    : gameOver(false),
      tickCounter(0),
      fallInterval(60),   // Tốc độ rơi mặc định 60 tick (~1 giây)
      currentScore(0),
      highScore(0),
      joystickHeld(false) // Khởi tạo cờ chống kẹt nút
{
    // Khởi tạo boardLogic trống
    for (int r = 0; r < BOARD_ROWS; ++r)
        for (int c = 0; c < BOARD_COLS; ++c)
            boardLogic[r][c] = 0;
}

// Khởi tạo màn hình, được Designer gọi khi màn hình được kích hoạt
void PlayScreenView::setupScreen()
{
    PlayScreenViewBase::setupScreen();

    // Khởi tạo 200 ô ảnh tĩnh vào board_container
    for (int r = 0; r < BOARD_ROWS; ++r)
    {
        for (int c = 0; c < BOARD_COLS; ++c)
        {
            gridImages[r][c].setXY(c * BLOCK_SIZE, r * BLOCK_SIZE);
            gridImages[r][c].setBitmap(touchgfx::Bitmap(BITMAP_BLOCK1_ID));
            gridImages[r][c].setVisible(false);
            board_container.add(gridImages[r][c]);
        }
    }
    board_container.invalidate();

    // Khởi tạo 16 ô ảnh tĩnh cho NEXT BLOCK
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            nextBlockImages[r][c].setXY(c * MINI_BLOCK_SIZE, r * MINI_BLOCK_SIZE);
            // Mặc định gán 1 ảnh bất kỳ, sẽ được update sau
            nextBlockImages[r][c].setBitmap(touchgfx::Bitmap(BITMAP_BLOCK1_MINI_ID));
            nextBlockImages[r][c].setVisible(false); // Ẩn đi khi chưa có khối
            next_block_container.add(nextBlockImages[r][c]);
        }
    }
    next_block_container.invalidate();

    // Ẩn các text tĩnh của Designer (sẽ thay thế bằng text với wildcard để hiển thị điểm số động)
    score.setVisible(false);
    highscore.setVisible(false);
    go_ys.setVisible(false);
    go_hs.setVisible(false);

    // Khởi tạo text với wildcard để hiển thị điểm số động.
    // score widget: pos(160,58,69,29), màu tím (154,82,255)
    scoreBuf[0] = '0'; scoreBuf[1] = 0;
    scoreWC.setPosition(160, 58, 69, 29);
    scoreWC.setColor(touchgfx::Color::getColorFromRGB(154, 82, 255));
    scoreWC.setLinespacing(0);
    scoreWC.setTypedText(touchgfx::TypedText(T_SCORE_WILDCARD));
    scoreWC.setWildcard(scoreBuf);
    add(scoreWC);

    // highscore widget: pos(160,115,69,28)
    hsBuf[0] = '0'; hsBuf[1] = 0;
    highscoreWC.setPosition(160, 115, 69, 28);
    highscoreWC.setColor(touchgfx::Color::getColorFromRGB(154, 82, 255));
    highscoreWC.setLinespacing(0);
    highscoreWC.setTypedText(touchgfx::TypedText(T_SCORE_WILDCARD));
    highscoreWC.setWildcard(hsBuf);
    add(highscoreWC);

    // go_ys (game over - your score) widget: pos(152,167,42,20), màu trắng
    goScoreBuf[0] = '0'; goScoreBuf[1] = 0;
    goYsWC.setPosition(152, 167, 42, 20);
    goYsWC.setColor(touchgfx::Color::getColorFromRGB(255, 255, 255));
    goYsWC.setLinespacing(0);
    goYsWC.setTypedText(touchgfx::TypedText(T_GO_SCORE_WILDCARD));
    goYsWC.setWildcard(goScoreBuf);
    goYsWC.setVisible(false);
    add(goYsWC);

    // go_hs (game over - high score) widget: pos(152,187,42,18)
    goHsBuf[0] = '0'; goHsBuf[1] = 0;
    goHsWC.setPosition(152, 187, 42, 18);
    goHsWC.setColor(touchgfx::Color::getColorFromRGB(255, 255, 255));
    goHsWC.setLinespacing(0);
    goHsWC.setTypedText(touchgfx::TypedText(T_GO_SCORE_WILDCARD));
    goHsWC.setWildcard(goHsBuf);
    goHsWC.setVisible(false);
    add(goHsWC);
}

void PlayScreenView::tearDownScreen()
{
    PlayScreenViewBase::tearDownScreen();
}

// startGame - Reset toàn bộ trạng thái, được Presenter gọi
void PlayScreenView::startGame()
{
    gameOver     = false;
    tickCounter  = 0;
    currentScore = 0;
    joystickHeld = false; // Reset cờ khi bắt đầu game mới

    // Thiết lập fallInterval ban đầu theo gameMode
    // EASY=80 ticks (~1.3s), MEDIUM=55 ticks (~0.9s), HARD=35 ticks (~0.6s)
    int gm = presenter->getGameMode();
    if      (gm == 2) fallInterval = 35; // HARD
    else if (gm == 1) fallInterval = 55; // MEDIUM
    else              fallInterval = 80; // EASY (default)

    // Lấy điểm cao nhất từ phiên trước
    highScore = presenter->getHighScore();

    // Xóa boardLogic
    for (int r = 0; r < BOARD_ROWS; ++r)
        for (int c = 0; c < BOARD_COLS; ++c)
            boardLogic[r][c] = 0;

    // Hiện lại điểm số của màn hình chơi
    scoreWC.setVisible(true);
    highscoreWC.setVisible(true);
    scoreWC.invalidate();
    highscoreWC.invalidate();

    // Ẩn overlay Game Over
    game_over.setVisible(false);
    replay_button.setVisible(false);
    text_go_ys.setVisible(false);
    goYsWC.setVisible(false);
    text_go_hs.setVisible(false);
    goHsWC.setVisible(false);
    game_over.invalidate();
    replay_button.invalidate();
    text_go_ys.invalidate();
    goYsWC.invalidate();
    text_go_hs.invalidate();
    goHsWC.invalidate();

    // Cập nhật điểm số ban đầu
    updateScoreUI();

    // Sinh ra khối NEXT đầu tiên trước khi game thực sự bắt đầu
    nextPieceType = getTetrisBagPiece();

    // Sinh khối đầu tiên
    spawnNewPiece();

    // Vẽ lại bảng
    updateBoardUI();

    Audio_StopSFX();
    Audio_StartBGM();
}

// Game Loop chính (~ 60 fps)
void PlayScreenView::handleTickEvent()
{
    if (gameOver) return; // Dừng game loop khi game kết thúc

    tickCounter++;

    // Mỗi fallInterval tick, khối rơi xuống 1 ô
    if (tickCounter >= fallInterval)
    {
        tickCounter = 0;
        movePiece(0, 1); // Dịch xuống 1 hàng
    }
}

// Nhận lệnh từ Presenter và xử lý
void PlayScreenView::onJoystickInput(uint8_t direction)
{
    if (gameOver) return;

    if (direction == 0)
    {
        // Joystick về IDLE => reset cờ, cho phép ROTATE lần tiếp
        joystickHeld = false;
        return;
    }

    switch (direction)
    {
        // Sang trái
        case 1:
            movePiece(-1, 0);
            break;

        // Sang phải
        case 2:
            movePiece( 1, 0);
            break;

        // Xoay khối theo chiều kim đồng hồ (chỉ xoay khi chưa giữ nút)
        case 3:
            if (!joystickHeld)
            {
                rotatePiece();
            }
            break;

        // Rơi nhanh
        case 4:
            movePiece( 0, 1);
            break;

        default:
            break;
    }

    // Đánh dấu joystick đang bị giữ
    joystickHeld = true;
}

// Sinh khối mới ngẫu nhiên ở đỉnh bảng
void PlayScreenView::spawnNewPiece()
{
    // 1. Lấy khối NEXT (đã sinh ra từ trước) làm khối hiện tại
    int typeIndex = nextPieceType;

    // Lấy hình dạng từ bảng TETROMINOES
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            currentPiece.shape[r][c] = TETROMINOES[typeIndex][r][c];

    currentPiece.type = typeIndex + 1;  // type = 1..7 (khớp với BLOCK_BITMAPS)
    currentPiece.x    = 3;              // Căn giữa theo chiều ngang (cột 3)
    currentPiece.y    = -1;             // Bắt đầu từ hàng trên cùng (-1, để khối rơi xuống hàng 0 ngay lần tick đầu tiên)

    // Ngay lập tức, sinh ra khối mới NEXT để chuẩn bị cho lần spawn tiếp theo
    nextPieceType = getTetrisBagPiece();
    updateNextBlockUI(); // Cập nhật ô Next Block

    // Nếu ngay khi sinh ra đã va chạm => Game Over
    if (checkCollision(currentPiece.x, currentPiece.y, currentPiece.shape))
    {
        gameOver = true;
        showGameOver();
    }
}

// Kiểm tra va chạm (True nếu va chạm) với tường, đáy, hoặc ô đã khoá
bool PlayScreenView::checkCollision(int nextX, int nextY, int checkShape[4][4])
{
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            // Bỏ qua ô trống trong ma trận khối
            if (checkShape[r][c] == 0) continue;

            // Tính vị trí ô trên boardLogic
            int boardCol = nextX + c;
            int boardRow = nextY + r;

            // Va chạm tường trái/phải
            if (boardCol < 0 || boardCol >= BOARD_COLS) return true;

            // Va chạm đáy
            if (boardRow >= BOARD_ROWS) return true;

            // Va chạm ô đã khoá (bỏ qua hàng âm - khối đang xuất hiện)
            if (boardRow >= 0 && boardLogic[boardRow][boardCol] != 0) return true;
        }
    }
    return false;
}

// Ghi khối vào boardLogic khi không thể rơi thêm
void PlayScreenView::lockPiece()
{
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            if (currentPiece.shape[r][c] == 0) continue;

            int boardRow = currentPiece.y + r;
            int boardCol = currentPiece.x + c;

            // Chỉ ghi vào boardLogic nếu ô nằm trong bảng (bỏ qua các ô trên cùng âm)
            if (boardRow >= 0 && boardRow < BOARD_ROWS &&
                boardCol >= 0 && boardCol < BOARD_COLS)
            {
                boardLogic[boardRow][boardCol] = currentPiece.type;
            }
        }
    }

    // Sau khi khoá, kiểm tra và xoá các hàng đầy, cộng điểm, sinh khối mới
    checkAndClearLines();
    spawnNewPiece();
    updateBoardUI();
}

// Nếu có hàng đầy, xoá đi và dịch các hàng phía trên xuống, cộng điểm
void PlayScreenView::checkAndClearLines()
{
    int linesCleared = 0;       // Biến đếm số hàng đã xoá, để tính điểm

    for (int r = BOARD_ROWS - 1; r >= 0; )
    {
        // Kiểm tra hàng r có đầy không
        bool full = true;

        for (int c = 0; c < BOARD_COLS; ++c)
        {
            // Nếu có ô trống => không đầy, trả về ngay
            if (boardLogic[r][c] == 0) { full = false; break; }
        }

        // Nếu đầy, xoá hàng r và dịch các hàng phía trên xuống 1 hàng
        if (full)
        {
            linesCleared++;

            // Dịch tất cả hàng phía trên xuống 1 hàng
            for (int moveRow = r; moveRow > 0; --moveRow)
                for (int c = 0; c < BOARD_COLS; ++c)
                    boardLogic[moveRow][c] = boardLogic[moveRow - 1][c];

            // Xoá hàng 0 (trên cùng)
            for (int c = 0; c < BOARD_COLS; ++c)
                boardLogic[0][c] = 0;

            // Giữ nguyên r để tiếp tục kiểm tra hàng r mới (những hàng phía trên đã dịch xuống)
            // trong trường hợp nhiều hàng liên tiếp đầy, ví dụ: 4 hàng cùng lúc
        }

        // Chỉ khi hàng r không đầy mới giảm r xuống để kiểm tra hàng phía trên
        else
        {
            --r;
        }
    }

    // Tính điểm theo mode:
    //   EASY   (0): 1x  -> 1 hàng=100, 2=300, 3=500, 4=800
    //   MEDIUM (1): 2x  -> 1 hàng=200, 2=600, 3=1000, 4=1600
    //   HARD   (2): 3x  -> 1 hàng=300, 2=900, 3=1500, 4=2400
    static const int SCORE_BASE[5] = {0, 100, 300, 500, 800};
    int gm = presenter->getGameMode();
    int multiplier = gm + 1; // EASY=1, MEDIUM=2, HARD=3

    if (linesCleared > 0 && linesCleared <= 4)
    {
        if (linesCleared == 4)
            Audio_PlaySFX(SFX_ROW_CLEAR_4);
        else
            Audio_PlaySFX(SFX_ROW_CLEAR);

        currentScore += SCORE_BASE[linesCleared] * multiplier;

        // Cập nhật highScore nếu vượt qua
        if (currentScore > highScore)
        {
            highScore = currentScore;
        }

        // Tăng tốc rơi theo mode:
        //   EASY:   fallInterval = 80 - (score/600)*5, min=30
        //   MEDIUM: fallInterval = 55 - (score/500)*5, min=20
        //   HARD:   fallInterval = 35 - (score/400)*7, min=10
        int baseInterval, scoreStep, stepSize, minInterval;
        if (gm == 2)      { baseInterval = 35; scoreStep = 400; stepSize = 7; minInterval = 10; }
        else if (gm == 1) { baseInterval = 55; scoreStep = 500; stepSize = 5; minInterval = 20; }
        else              { baseInterval = 80; scoreStep = 600; stepSize = 5; minInterval = 30; }

        fallInterval = baseInterval - (currentScore / scoreStep) * stepSize;
        if (fallInterval < minInterval) fallInterval = minInterval;

        updateScoreUI();
    }
}

// Dịch chuyển khối (deltaX = ngang, deltaY = dọc)
void PlayScreenView::movePiece(int deltaX, int deltaY)
{
    int newX = currentPiece.x + deltaX;
    int newY = currentPiece.y + deltaY;

    if (!checkCollision(newX, newY, currentPiece.shape))
    {
        // Không va chạm => di chuyển được
        currentPiece.x = newX;
        currentPiece.y = newY;
        updateBoardUI();
        if (deltaX != 0) Audio_PlaySFX(SFX_MOVE);
    }
    else if (deltaY > 0)
    {
        // Va chạm khi đang rơi xuống => khoá khối vào bảng
        lockPiece();
    }
    // Va chạm ngang => bỏ qua (không di chuyển)
}

// Xoay 90° theo chiều kim đồng hồ
void PlayScreenView::rotatePiece()
{
    // Tạo ma trận xoay tạm
    int rotated[4][4] = {};

    // Phép xoay 90 độ theo chiều kim đồng hồ: rotated[c][3-r] = shape[r][c]
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            rotated[c][3 - r] = currentPiece.shape[r][c];

    // Kiểm tra va chạm sau xoay
    if (!checkCollision(currentPiece.x, currentPiece.y, rotated))
    {
        // Không va chạm => áp dụng xoay
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                currentPiece.shape[r][c] = rotated[r][c];

        updateBoardUI();
        Audio_PlaySFX(SFX_ROTATE);
        return;
    }

    // Wall-kick đơn giản: thử dịch trái/phải 1 ô rồi xoay
    // Thử dịch phải 1
    if (!checkCollision(currentPiece.x + 1, currentPiece.y, rotated))
    {
        currentPiece.x += 1;
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                currentPiece.shape[r][c] = rotated[r][c];
        updateBoardUI();
        Audio_PlaySFX(SFX_ROTATE);
        return;
    }

    // Thử dịch trái 1
    if (!checkCollision(currentPiece.x - 1, currentPiece.y, rotated))
    {
        currentPiece.x -= 1;
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                currentPiece.shape[r][c] = rotated[r][c];
        updateBoardUI();
        Audio_PlaySFX(SFX_ROTATE);
        return;
    }
    // Không thể xoay => bỏ qua
}

// Vẽ lại toàn bộ lưới (bảng + khối đang rơi)
void PlayScreenView::updateBoardUI()
{
    // 1. Tạo snapshot hợp nhất (boardLogic + currentPiece)
    // dùng mảng tạm để không sửa boardLogic gốc
    int displayBoard[BOARD_ROWS][BOARD_COLS];

    for (int r = 0; r < BOARD_ROWS; ++r)
        for (int c = 0; c < BOARD_COLS; ++c)
            displayBoard[r][c] = boardLogic[r][c];

    // Vẽ khối đang rơi vào snapshot
    if (!gameOver)
    {
        for (int r = 0; r < 4; ++r)
        {
            for (int c = 0; c < 4; ++c)
            {
                if (currentPiece.shape[r][c] == 0) continue;

                int boardRow = currentPiece.y + r;
                int boardCol = currentPiece.x + c;

                if (boardRow >= 0 && boardRow < BOARD_ROWS &&
                    boardCol >= 0 && boardCol < BOARD_COLS)
                {
                    displayBoard[boardRow][boardCol] = currentPiece.type;
                }
            }
        }
    }

    // 2. Đồng bộ displayBoard => gridImages
    for (int r = 0; r < BOARD_ROWS; ++r)
    {
        for (int c = 0; c < BOARD_COLS; ++c)
        {
            int blockType = displayBoard[r][c];

            if (blockType == 0)
            {
                gridImages[r][c].setVisible(false);
            }
            else
            {
                gridImages[r][c].setBitmap(
                    touchgfx::Bitmap(BLOCK_BITMAPS[blockType])
                );
                gridImages[r][c].setVisible(true);
            }
        }
    }

    board_container.invalidate();
}

// Cập nhật text_score và text_highscore
void PlayScreenView::updateScoreUI()
{
    // Cập nhật scoreWC
    touchgfx::Unicode::snprintf(scoreBuf, 12, "%d", currentScore);
    scoreWC.invalidate();

    // Cập nhật highscoreWC
    touchgfx::Unicode::snprintf(hsBuf, 12, "%d", highScore);
    highscoreWC.invalidate();
}

// Hiện overlay Game Over và cập nhật điểm số thua
void PlayScreenView::showGameOver()
{
    Audio_StopBGM();
    Audio_PlaySFX(SFX_GAME_OVER);
    // Lưu điểm số cao nhất vào Presenter để lưu vào Model 
    presenter->saveHighScore(highScore);

    // Ẩn điểm số của màn hình đang chơi
    scoreWC.setVisible(false);
    highscoreWC.setVisible(false);
    scoreWC.invalidate();
    highscoreWC.invalidate();

    // Hiện overlay
    game_over.setVisible(true);
    replay_button.setVisible(true);
    text_go_ys.setVisible(true);
    text_go_hs.setVisible(true);
    goYsWC.setVisible(true);
    goHsWC.setVisible(true);

    // Cập nhật điểm trên màn hình Game Over
    touchgfx::Unicode::snprintf(goScoreBuf, 12, "%d", currentScore);
    goYsWC.invalidate();

    touchgfx::Unicode::snprintf(goHsBuf, 12, "%d", highScore);
    goHsWC.invalidate();

    // Invalidate phần còn lại
    game_over.invalidate();
    replay_button.invalidate();
    text_go_ys.invalidate();
    text_go_hs.invalidate();
}

// Vẽ khối tiếp theo lên ô Next Block bên phải bảng chính
void PlayScreenView::updateNextBlockUI()
{
    // Cộng 1 để khớp với MINI_BLOCK_BITMAPS (index 0 không dùng)
    int typeColor = nextPieceType + 1; 

    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            // Kiểm tra xem tại ô [r][c] của khối này có gạch hay không
            if (TETROMINOES[nextPieceType][r][c] == 0)
            {
                nextBlockImages[r][c].setVisible(false);
            }
            else
            {
                // Gán đúng Bitmap ID từ mảng MINI_BLOCK_BITMAPS
                nextBlockImages[r][c].setBitmap(touchgfx::Bitmap(MINI_BLOCK_BITMAPS[typeColor]));
                nextBlockImages[r][c].setVisible(true);
            }
        }
    }

    next_block_container.invalidate();
}

// Callback từ nút Replay (gắn qua Designer)
void PlayScreenView::replayGameClicked()
{
    startGame(); // Reset toàn bộ => game mới bắt đầu
}