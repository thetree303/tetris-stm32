#ifndef PLAYSCREENVIEW_HPP
#define PLAYSCREENVIEW_HPP

#include <gui/playscreen_screen/PlayScreenPresenter.hpp>
#include <gui_generated/playscreen_screen/PlayScreenViewBase.hpp>
#include <touchgfx/widgets/TextAreaWithWildcard.hpp>

// Cấu trúc dữ liệu cho một khối
struct Piece {
  int shape[4][4]; // Ma trận hình dạng khối
  int type;        // Loại khối (1-7) để lấy màu sắc tương ứng
  int x;           // Cột của ô trên-trái ([0][0]) ma trận 4x4
  int y;           // Hàng của ô trên-trái ([0][0]) ma trận 4x4
};

class PlayScreenView : public PlayScreenViewBase {
public:
  PlayScreenView();
  virtual ~PlayScreenView() {}
  virtual void setupScreen();
  virtual void tearDownScreen();

  // --------------------------------------------------------
  // PUBLIC API - được gọi từ Presenter
  // --------------------------------------------------------

  // Bắt đầu game mới (reset trạng thái)
  void startGame();

  // Nhận lệnh Joystick từ Presenter và xử lý
  void onJoystickInput(uint8_t direction);

  // --------------------------------------------------------
  // VIRTUAL FUNCTION từ ViewBase (được Designer nối với nút)
  // --------------------------------------------------------
  virtual void replayGameClicked();

protected:
  // --------------------------------------------------------
  // HẰNG SỐ KÍCH THƯỚC BÀN CỜ
  // --------------------------------------------------------
  static const int BOARD_COLS = 10; // Số cột
  static const int BOARD_ROWS = 20; // Số hàng
  static const int BLOCK_SIZE = 12; // Số pixel mỗi ô (khớp board_container 120x240)

  // --------------------------------------------------------
  // TRẠNG THÁI GAME LOOP
  // --------------------------------------------------------
  bool gameOver;     // Cờ game kết thúc
  int tickCounter;   // Đếm tick từ TouchGFX
  int fallInterval;  // Số tick giữa mỗi lần rơi tự động
  int currentScore;  // Điểm hiện tại
  int highScore;     // Điểm cao nhất
  bool joystickHeld; // TRUE khi joystick đang bị giữ (chưa nhả)

  // --------------------------------------------------------
  // DỮ LIỆU LOGIC BÀN CỜ
  // --------------------------------------------------------
  // 0 = ô trống, 1-7 = ô đã bị khóa (mang loại màu tương ứng)
  int boardLogic[BOARD_ROWS][BOARD_COLS];

  // --------------------------------------------------------
  // ĐỒ HỌA BÀN CỜ - 200 ô Image được thêm vào board_container
  // --------------------------------------------------------
  touchgfx::Image gridImages[BOARD_ROWS][BOARD_COLS];

  // --------------------------------------------------------
  // KHỐI ĐANG RƠI
  // --------------------------------------------------------
  Piece currentPiece;

  // --------------------------------------------------------
  // KHỐI TIẾP THEO
  // --------------------------------------------------------
  int nextPieceType;
  touchgfx::Image nextBlockImages[4][4];
  static const int MINI_BLOCK_SIZE = 9;  // Kích thước mỗi ô trong ô Next Block (4x4)

  // --------------------------------------------------------
  // GAME LOOP - Override từ TouchGFX View
  // Được gọi tự động mỗi frame (~60 fps)
  // --------------------------------------------------------
  virtual void handleTickEvent();

  // --------------------------------------------------------
  // HÀM GAME LOGIC NỘI BỘ
  // --------------------------------------------------------

  /** Sinh ngẫu nhiên khối mới ở đỉnh bảng */
  void spawnNewPiece();

  /** Kiểm tra va chạm: tường, đáy, ô đã khoá */
  bool checkCollision(int nextX, int nextY, int checkShape[4][4]);

  /** Khoá khối vào boardLogic khi chạm đáy/vật cản */
  void lockPiece();

  /** Kiểm tra & xoá hàng đầy, cộng điểm */
  void checkAndClearLines();

  /** Dịch chuyển khối theo delta (deltaX, deltaY) */
  void movePiece(int deltaX, int deltaY);

  /** Xoay khối 90° theo chiều kim đồng hồ */
  void rotatePiece();
  
  // --------------------------------------------------------
  // HÀM GIAO DIỆN
  // --------------------------------------------------------

  /** Vẽ lại toàn bộ bảng (boardLogic + currentPiece) lên gridImages */
  void updateBoardUI();

  /** Cập nhật ô Next Block */
  void updateNextBlockUI();

  /** Hiển thị overlay Game Over */
  void showGameOver();

  /** Cập nhật TextArea điểm số */
  void updateScoreUI();

  // --------------------------------------------------------
  // WILDCARD TEXT AREAS cho điểm số động
  // (Thay thế các TextArea tĩnh của ViewBase)
  // --------------------------------------------------------
  touchgfx::TextAreaWithOneWildcard scoreWC;     // Điểm hiện tại
  touchgfx::TextAreaWithOneWildcard highscoreWC; // Điểm cao nhất
  touchgfx::TextAreaWithOneWildcard goYsWC;      // Điểm Game Over
  touchgfx::TextAreaWithOneWildcard goHsWC;      // High Score Game Over

  // Buffer lưu chuỗi số (UTF-16 của TouchGFX)
  touchgfx::Unicode::UnicodeChar scoreBuf[12];
  touchgfx::Unicode::UnicodeChar hsBuf[12];
  touchgfx::Unicode::UnicodeChar goScoreBuf[12];
  touchgfx::Unicode::UnicodeChar goHsBuf[12];
};

#endif // PLAYSCREENVIEW_HPP
