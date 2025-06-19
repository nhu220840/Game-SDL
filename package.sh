#!/bin/bash

#-----------------------------------------------------------------------
# SCRIPT TỰ ĐỘNG ĐÓNG GÓI GAME SDL2 SHOOTER CHO LINUX
#-----------------------------------------------------------------------

# Dừng script ngay lập tức nếu có lệnh nào thất bại
set -e

# --- CÁC BIẾN CẤU HÌNH ---
TARGET_EXEC="shooter_game"
RELEASE_DIR="shooter_game_release"
FINAL_ARCHIVE="shooter_game_linux_$(date +%Y-%m-%d).tar.gz"

# --- BẮT ĐẦU QUÁ TRÌNH ---

echo "========================================="
echo "Bắt đầu quá trình đóng gói game..."
echo "========================================="

# 1. Biên dịch phiên bản mới nhất của game
echo ">>> Bước 1: Biên dịch game..."
if [ ! -f Makefile ]; then
    echo "Lỗi: Không tìm thấy Makefile. Vui lòng chạy script này từ thư mục gốc của dự án."
    exit 1
fi
make clean
make
echo "Biên dịch thành công."

# 2. Dọn dẹp và tạo lại thư mục phát hành
echo ">>> Bước 2: Chuẩn bị thư mục phát hành..."
rm -rf "$RELEASE_DIR"
mkdir "$RELEASE_DIR"
echo "Thư mục '$RELEASE_DIR' đã được tạo."

# 3. Sao chép các tệp cần thiết
echo ">>> Bước 3: Sao chép tệp thực thi và tài nguyên..."
cp "$TARGET_EXEC" "$RELEASE_DIR/"
cp -r assets/ "$RELEASE_DIR/"
echo "Đã sao chép tệp game và thư mục assets."

# 4. Tìm và sao chép các thư viện SDL cần thiết
echo ">>> Bước 4: Tìm và đóng gói các thư viện SDL..."
# Sử dụng ldd để tìm các thư viện, sau đó lọc ra những thư viện cần thiết (SDL, png, jpg, ogg, etc.)
# và sao chép chúng vào thư mục phát hành.
ldd "$TARGET_EXEC" | grep -E 'libSDL2|libpng|libjpeg|libvorbis|libogg|libmpg123' | awk '{print $3}' | while read -r lib_path ; do
    if [ -f "$lib_path" ]; then
        echo "   Đóng gói: $(basename "$lib_path")"
        cp "$lib_path" "$RELEASE_DIR/"
    else
        echo "   Cảnh báo: Không tìm thấy thư viện '$lib_path'"
    fi
done
echo "Đóng gói thư viện hoàn tất."

# 5. Tạo script khởi động cho người chơi
echo ">>> Bước 5: Tạo script 'run.sh'..."
cat << EOF > "$RELEASE_DIR/run.sh"
#!/bin/bash
# Di chuyển đến thư mục chứa script để đảm bảo đường dẫn tài nguyên luôn đúng
cd "\$(dirname "\$0")"

# Thiết lập biến môi trường để chương trình ưu tiên tìm thư viện trong thư mục này
export LD_LIBRARY_PATH=.

# Chạy game
./$TARGET_EXEC
EOF
chmod +x "$RELEASE_DIR/run.sh"
echo "Đã tạo run.sh và cấp quyền thực thi."

# 6. Nén toàn bộ thư mục thành tệp .tar.gz
echo ">>> Bước 6: Nén gói phân phối..."
tar -czvf "$FINAL_ARCHIVE" "$RELEASE_DIR"

# --- KẾT THÚC ---
echo "========================================="
echo "Hoàn tất!"
echo "Gói phân phối của bạn đã được tạo: $FINAL_ARCHIVE"
echo "========================================="