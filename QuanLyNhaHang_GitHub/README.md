# Ứng dụng quản lý nhà hàng — Cấu trúc dữ liệu và giải thuật

Đồ án minh họa nghiệp vụ **quản lý nhà hàng** và ứng dụng cấu trúc dữ liệu, giải thuật trên **C++17 (Console)**, kèm **giao diện web HTML/CSS/JavaScript** để trình diễn luồng nghiệp vụ.

> **Lưu ý kiến trúc:** `web/` và `console/` là **hai bản demo độc lập**. Giao diện web dùng JavaScript và `localStorage`, **chưa kết nối** với chương trình C++ và **không thực thi** các thuật toán C++ bên dưới. Khi báo cáo các thuật toán, hãy chạy ứng dụng console, kiểm thử và benchmark trong `console/`.

## Tính năng

| Nhóm chức năng | Bản web (`web/`) | Bản console (`console/`) |
|---|---|---|
| Quản lý bàn, thực đơn | Có | Có |
| Tạo đơn, gọi món, thanh toán | Có | Có |
| Thống kê doanh thu | Có | Có |
| Tìm món theo mã bằng Binary Search | Không | Có |
| Sắp xếp bằng Merge Sort | Không | Có |
| Gợi ý combo theo ngân sách (0/1 Knapsack) | Không | Có |
| Lưu dữ liệu | `localStorage` | File văn bản `.db` trong `console/data/` |

## Cấu trúc thư mục

```text
QuanLyNhaHang_GitHub/
├── README.md                  # Giới thiệu và hướng dẫn
├── .gitignore                 # Bỏ qua file build và dữ liệu chạy
├── web/
│   └── index.html            # Giao diện demo: HTML, CSS, JavaScript
├── console/
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── algorithms.hpp    # Binary Search, Merge Sort, 0/1 Knapsack
│   │   └── restaurant.hpp    # Các cấu trúc và khai báo nghiệp vụ
│   ├── src/
│   │   ├── main.cpp          # Menu console
│   │   ├── restaurant.cpp    # Logic quản lý nhà hàng
│   │   └── benchmark.cpp     # Đo thời gian giải thuật
│   ├── tests/
│   │   └── test_algorithms.cpp
│   ├── benchmarks/
│   │   ├── results.csv       # Kết quả đo tham khảo
│   │   ├── environment.txt
│   │   └── demo_output.txt
│   └── data/
│       └── README.txt        # Dữ liệu .db được tạo khi chạy
└── docs/
    ├── BaoCao_QuanLyNhaHang.docx
    ├── BaoCao_QuanLyNhaHang.pdf
    ├── Slides_QuanLyNhaHang.pptx
    └── Slides_QuanLyNhaHang.pdf
```

## Chạy giao diện web

Mở `web/index.html` bằng Chrome hoặc Edge, không cần cài thư viện.

- Có thể thao tác bàn, món ăn, đơn hàng, thanh toán và báo cáo.
- Trình duyệt lưu thông tin vào `localStorage` trên chính máy đang dùng; đây là dữ liệu demo, không phải cơ sở dữ liệu dùng chung.

## Chạy chương trình C++ (Windows / Linux / macOS)

Yêu cầu: **trình biên dịch hỗ trợ C++17**, ví dụ `g++` (MinGW-w64 trên Windows).

Từ thư mục gốc của repository, chuyển vào thư mục console:

```bash
cd console
g++ -std=c++17 -O2 -Iinclude src/main.cpp src/restaurant.cpp -o restaurant_app
```

Chạy:

```powershell
# Windows (PowerShell)
.\restaurant_app.exe
```

```bash
# Linux / macOS
./restaurant_app
```

Nên chạy từ thư mục `console/` để file dữ liệu được đọc/ghi đúng vào `console/data/`.

### Biên dịch bằng CMake (tùy chọn)

```bash
cd console
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

Với Visual Studio hoặc trình biên dịch đa cấu hình, file chạy có thể nằm trong `build/Release/`.

## Ba thuật toán chính

| Thuật toán | Mục đích | Độ phức tạp lý thuyết | Mã nguồn |
|---|---|---|---|
| Binary Search | Tìm món theo mã trên danh sách đã sắp xếp | `O(log n)` mỗi lần tìm; chưa tính sắp xếp | `console/include/algorithms.hpp` |
| Merge Sort | Sắp xếp món theo giá hoặc theo mã | `O(n log n)` thời gian, `O(n)` bộ nhớ phụ | `console/include/algorithms.hpp` |
| 0/1 Knapsack (Quy hoạch động) | Gợi ý tối đa tổng điểm yêu thích trong ngân sách | `O(nB)` thời gian; `B` tính theo nghìn đồng | `console/include/algorithms.hpp` |

Các cấu trúc dữ liệu gồm `std::vector<Dish>`, `std::vector<Table>`, `std::vector<Order>`, `std::vector<OrderItem>`, `std::vector<Receipt>` và mảng quy hoạch động. Xem `console/include/restaurant.hpp`.

## Chạy kiểm thử và benchmark

Từ `console/`:

```bash
g++ -std=c++17 -O2 -Iinclude tests/test_algorithms.cpp -o test_algorithms
g++ -std=c++17 -O2 -Iinclude src/benchmark.cpp -o benchmark
```

Windows PowerShell:

```powershell
.\test_algorithms.exe
.\benchmark.exe > benchmarks/results_new.csv
```

Linux / macOS:

```bash
./test_algorithms
./benchmark > benchmarks/results_new.csv
```

Kết quả tham khảo ở `console/benchmarks/results.csv`, kèm thông tin môi trường ở `environment.txt`. Khi thuyết trình, hãy **chạy lại benchmark trên máy của mình**; thời gian chạy phụ thuộc CPU, compiler và cách đo. Các phép đo tìm kiếm tách riêng chi phí sắp xếp tiền xử lý.

## Quy trình demo gợi ý

1. Mở `web/index.html` để minh họa các thao tác nhà hàng.
2. Chạy `console/restaurant_app` để tạo đơn và thanh toán qua menu console.
3. Tìm món theo mã (Binary Search); sắp xếp món theo giá (Merge Sort).
4. Nhập ngân sách để nhận gợi ý combo (0/1 Knapsack).
5. Chạy `test_algorithms` và `benchmark`, đối chiếu kết quả với phần phân tích trong báo cáo.

## Cách đưa lên GitHub

1. Trên GitHub, chọn **New repository**, đặt tên `restaurant-management`, chọn Public hoặc Private; **không** chọn khởi tạo README/.gitignore vì repository đã có sẵn.
2. Mở terminal tại thư mục **`QuanLyNhaHang_GitHub/`** và chạy:

```bash
git init
git add .
git commit -m "Initial commit: restaurant management app and algorithms"
git branch -M main
git remote add origin https://github.com/USERNAME/restaurant-management.git
git push -u origin main
```

Thay `USERNAME` bằng tên tài khoản GitHub thực tế. Nếu Git yêu cầu, hãy xác thực bằng trình duyệt/Git Credential Manager hoặc GitHub CLI; không dùng mật khẩu tài khoản GitHub trong terminal. Git có thể yêu cầu cấu hình `user.name` và `user.email` trước khi commit.

## Giới hạn

- Bản web chưa liên thông bản C++; chưa có backend hoặc đồng bộ đa thiết bị.
- Bản console là ứng dụng đơn người dùng, lưu text local (đuôi `.db` **không phải SQLite**).
- Thuật toán 0/1 Knapsack chỉ gợi ý mỗi món tối đa một lần; chưa tính khẩu phần, dị ứng hay nhóm món.
- Chưa có phân quyền, quản lý tồn kho hoặc tích hợp thanh toán trực tuyến.

## Hồ sơ nộp bài

- Báo cáo: `docs/BaoCao_QuanLyNhaHang.docx` / `.pdf`
- Slides: `docs/Slides_QuanLyNhaHang.pptx` / `.pdf`
- Source code: `web/` + `console/`
- Kết quả thực nghiệm: `console/benchmarks/`
