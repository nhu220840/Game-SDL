#!/bin/bash

# SCRIPT TỰ ĐỘNG ĐÓNG GÓI GAME CHO WINDOWS

set -e # Dừng script nếu có lỗi

# --- CÁC BIẾN CẤU HÌNH ---
TARGET_EXEC="shooter_game.exe"
RELEASE_DIR="windows_release"
FINAL_ARCHIVE="shooter_game_windows_v1.0.zip"

# THAY ĐỔI 'your_username' THÀNH TÊN NGƯỜI DÙNG CỦA BẠN
MINGW_TOOLCHAIN_PATH="/home/sherry/mingw_toolchain/x86_64-w64-mingw32"

# --- BẮT ĐẦU ---
echo "========================================="
echo "Bắt đầu đóng gói phiên bản Windows..."
echo "========================================="

# 1. Biên dịch chéo phiên bản mới nhất
echo ">>> Bước 1: Biên dịch lại game cho Windows..."
make -f Makefile.windows clean
make -f Makefile.windows
echo "Biên dịch thành công."

# 2. Chuẩn bị thư mục phát hành
echo ">>> Bước 2: Chuẩn bị thư mục phát hành..."
rm -rf "$RELEASE_DIR"
mkdir "$RELEASE_DIR"

# 3. Sao chép các tệp cần thiết
echo ">>> Bước 3: Sao chép tệp game, assets và DLLs..."
cp "$TARGET_EXEC" "$RELEASE_DIR/"
cp -r assets/ "$RELEASE_DIR/"
cp "$MINGW_TOOLCHAIN_PATH/bin/"*.dll "$RELEASE_DIR/"
echo "Sao chép hoàn tất."

# 4. Nén thành tệp .zip
echo ">>> Bước 4: Nén thành tệp .zip..."
# Di chuyển vào thư mục release để cấu trúc file zip được đẹp
cd "$RELEASE_DIR"
zip -r ../"$FINAL_ARCHIVE" .
cd ..

# --- KẾT THÚC ---
echo "========================================="
echo "Hoàn tất!"
echo "Gói phân phối cho Windows đã được tạo: $FINAL_ARCHIVE"
echo "========================================="