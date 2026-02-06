import tkinter as tk
from tkinter import ttk, filedialog, messagebox
from PIL import Image, ImageTk
import requests
import json
import io
import threading
import os

# 配置
DEFAULT_API_URL = "http://localhost:8000/api/v1"
DEFAULT_DEVICE_ID = "sumhello"
DEFAULT_SECRET = "123456"

# 墨水屏尺寸常量
EPD_WIDTH = 212
EPD_HEIGHT = 104
EPD_ASPECT_RATIO = EPD_HEIGHT / EPD_WIDTH  # 0.4906

class InfinityTagClient(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Infinity Tag EPD 图片上传测试工具")
        self.geometry("1100x800")

        self.api_url = DEFAULT_API_URL
        self.token = None
        self.original_image = None  # PIL Image
        self.original_image_path = None
        self.tk_original_image = None
        self.preview_image_bytes = None
        self.tk_preview_image = None

        # 裁剪相关
        self.crop_start_x = 0
        self.crop_start_y = 0
        self.rect_id = None
        self.crop_coords = None # (x, y, w, h) based on original image

        # 显示缩放比例 (为了在 Canvas 上完整显示大图)
        self.display_scale = 1.0

        self._init_ui()
        self._show_login_dialog()

    def _init_ui(self):
        # === 顶部工具栏 ===
        toolbar = ttk.Frame(self, padding=10)
        toolbar.pack(fill=tk.X)

        ttk.Button(toolbar, text="选择图片", command=self.load_image).pack(side=tk.LEFT, padx=5)
        self.btn_upload = ttk.Button(toolbar, text="确认上传", command=self.upload_image, state=tk.DISABLED)
        self.btn_upload.pack(side=tk.LEFT, padx=5)

        self.lbl_status = ttk.Label(toolbar, text="请先登录", foreground="red")
        self.lbl_status.pack(side=tk.RIGHT, padx=5)

        # === 主区域 (左右分栏) ===
        main_pane = ttk.PanedWindow(self, orient=tk.HORIZONTAL)
        main_pane.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        # --- 左侧：编辑区 ---
        left_frame = ttk.LabelFrame(main_pane, text="原图编辑 (拖拽裁剪)", padding=10)
        main_pane.add(left_frame, weight=1)

        # 画布容器
        self.canvas_frame = ttk.Frame(left_frame)
        self.canvas_frame.pack(fill=tk.BOTH, expand=True)

        self.canvas = tk.Canvas(self.canvas_frame, bg="#e0e0e0", cursor="cross")
        self.canvas.pack(fill=tk.BOTH, expand=True)
        self.canvas.bind("<ButtonPress-1>", self.on_crop_start)
        self.canvas.bind("<B1-Motion>", self.on_crop_drag)
        self.canvas.bind("<ButtonRelease-1>", self.on_crop_end)

        # 参数控制区
        controls_frame = ttk.Frame(left_frame, padding="0 10 0 0")
        controls_frame.pack(fill=tk.X)

        # 旋转
        ttk.Label(controls_frame, text="旋转:").grid(row=0, column=0, padx=5)
        self.var_rotate = tk.IntVar(value=0)
        rotation_frame = ttk.Frame(controls_frame)
        rotation_frame.grid(row=0, column=1, columnspan=3, sticky="w")
        for deg in [0, 90, 180, 270]:
            ttk.Radiobutton(rotation_frame, text=f"{deg}°", variable=self.var_rotate, value=deg, command=self.trigger_preview).pack(side=tk.LEFT)

        # 反色
        self.var_invert = tk.BooleanVar(value=False)
        ttk.Checkbutton(controls_frame, text="反色 (Invert)", variable=self.var_invert, command=self.trigger_preview).grid(row=0, column=4, padx=10)

        # 模式 (抖动 vs 阈值)
        ttk.Label(controls_frame, text="模式:").grid(row=1, column=0, padx=5, pady=5)
        self.var_dither = tk.BooleanVar(value=True)
        ttk.Radiobutton(controls_frame, text="抖动 (照片)", variable=self.var_dither, value=True, command=self.toggle_mode).grid(row=1, column=1, sticky="w")
        ttk.Radiobutton(controls_frame, text="阈值 (文字)", variable=self.var_dither, value=False, command=self.toggle_mode).grid(row=1, column=2, sticky="w")

        # 阈值滑块
        self.lbl_threshold = ttk.Label(controls_frame, text="阈值 (128):")
        self.lbl_threshold.grid(row=1, column=3, padx=5)
        self.scale_threshold = ttk.Scale(controls_frame, from_=0, to=255, orient=tk.HORIZONTAL, command=self.on_threshold_change)
        self.scale_threshold.set(128)
        self.scale_threshold.grid(row=1, column=4, sticky="ew")

        # 预览按钮
        ttk.Button(controls_frame, text="刷新预览", command=self.trigger_preview).grid(row=0, column=5, rowspan=2, padx=10, sticky="ns")

        # --- 右侧：预览区 ---
        right_frame = ttk.LabelFrame(main_pane, text="墨水屏预览 (212x104)", padding=10)
        main_pane.add(right_frame, weight=1)

        self.preview_label = ttk.Label(right_frame, text="暂无预览", anchor="center", background="#ffffff")
        self.preview_label.pack(fill=tk.BOTH, expand=True)

        self.toggle_mode() # 初始化控件状态

    def _show_login_dialog(self):
        dialog = tk.Toplevel(self)
        dialog.title("登录")
        dialog.geometry("300x200")
        dialog.transient(self)
        dialog.grab_set()

        ttk.Label(dialog, text="Device ID:").pack(pady=5)
        entry_did = ttk.Entry(dialog)
        entry_did.insert(0, DEFAULT_DEVICE_ID)
        entry_did.pack(pady=5)

        ttk.Label(dialog, text="Secret:").pack(pady=5)
        entry_pwd = ttk.Entry(dialog, show="*")
        entry_pwd.insert(0, DEFAULT_SECRET)
        entry_pwd.pack(pady=5)

        def do_login():
            did = entry_did.get()
            pwd = entry_pwd.get()
            try:
                resp = requests.post(f"{self.api_url}/auth/login", json={"device_id": did, "password": pwd})
                if resp.status_code == 200:
                    self.token = resp.json()["access_token"]
                    self.lbl_status.config(text=f"已登录: {did}", foreground="green")
                    dialog.destroy()
                else:
                    messagebox.showerror("登录失败", resp.text)
            except Exception as e:
                messagebox.showerror("连接错误", str(e))

        ttk.Button(dialog, text="登录", command=do_login).pack(pady=10)
        self.wait_window(dialog)

    def load_image(self):
        file_path = filedialog.askopenfilename(filetypes=[("Images", "*.png;*.jpg;*.jpeg;*.bmp")])
        if not file_path:
            return

        self.original_image_path = file_path
        self.original_image = Image.open(file_path)
        self.crop_coords = None # 重置裁剪
        self.display_image_on_canvas()
        self.trigger_preview()
        self.btn_upload.config(state=tk.NORMAL)

    def display_image_on_canvas(self):
        if not self.original_image:
            return

        # 计算适合 Canvas 的缩放比例
        w, h = self.original_image.size
        cw = self.canvas.winfo_width() or 500
        ch = self.canvas.winfo_height() or 400

        scale_w = cw / w
        scale_h = ch / h
        self.display_scale = min(scale_w, scale_h, 1.0) # 只缩小不放大

        new_w = int(w * self.display_scale)
        new_h = int(h * self.display_scale)

        resized = self.original_image.resize((new_w, new_h), Image.Resampling.LANCZOS)
        self.tk_original_image = ImageTk.PhotoImage(resized)

        self.canvas.delete("all")
        # 居中显示
        x = (cw - new_w) // 2
        y = (ch - new_h) // 2
        self.image_offset_x = x
        self.image_offset_y = y

        self.canvas.create_image(x, y, image=self.tk_original_image, anchor="nw")
        self.canvas.config(scrollregion=self.canvas.bbox("all"))

    # === 裁剪交互 ===
    def on_crop_start(self, event):
        if not self.original_image: return
        self.crop_start_x = event.x
        self.crop_start_y = event.y
        if self.rect_id:
            self.canvas.delete(self.rect_id)
        self.rect_id = self.canvas.create_rectangle(self.crop_start_x, self.crop_start_y, self.crop_start_x, self.crop_start_y, outline="red", width=2, dash=(4, 4))

    def on_crop_drag(self, event):
        if not self.rect_id: return

        # 强制宽高比 212:104
        start_x, start_y = self.crop_start_x, self.crop_start_y
        current_x, current_y = event.x, event.y

        width = abs(current_x - start_x)
        height = width * EPD_ASPECT_RATIO

        # 根据拖动方向计算新的 y 坐标
        if current_y < start_y:
            end_y = start_y - height
        else:
            end_y = start_y + height

        # 根据拖动方向计算新的 x 坐标 (保持 x 方向跟随鼠标)
        if current_x < start_x:
            end_x = start_x - width
        else:
            end_x = start_x + width

        self.canvas.coords(self.rect_id, start_x, start_y, end_x, end_y)

    def on_crop_end(self, event):
        if not self.rect_id or not self.original_image: return

        coords = self.canvas.coords(self.rect_id)
        x1, y1, x2, y2 = coords

        # 规范化坐标 (确保 x1 < x2)
        x_min, x_max = sorted([x1, x2])
        y_min, y_max = sorted([y1, y2])

        # 转换为原图坐标
        # 减去 Canvas 偏移
        img_x = x_min - self.image_offset_x
        img_y = y_min - self.image_offset_y
        img_w = x_max - x_min
        img_h = y_max - y_min

        # 转换为真实像素坐标
        real_x = int(img_x / self.display_scale)
        real_y = int(img_y / self.display_scale)
        real_w = int(img_w / self.display_scale)
        real_h = int(img_h / self.display_scale)

        # 边界检查
        if real_w < 10 or real_h < 10: # 太小忽略
            self.canvas.delete(self.rect_id)
            self.rect_id = None
            self.crop_coords = None
        else:
            self.crop_coords = (real_x, real_y, real_w, real_h)
            self.trigger_preview()

    def toggle_mode(self):
        is_dither = self.var_dither.get()
        if is_dither:
            self.scale_threshold.state(['disabled'])
        else:
            self.scale_threshold.state(['!disabled'])
        self.trigger_preview()

    def on_threshold_change(self, val):
        self.lbl_threshold.config(text=f"阈值 ({int(float(val))}):")
        # 防抖可以在这里做，这里简单起见直接调用
        self.trigger_preview()

    def get_options(self):
        opts = {
            "rotate": self.var_rotate.get(),
            "invert": self.var_invert.get(),
            "dither": self.var_dither.get(),
            "threshold": int(self.scale_threshold.get())
        }
        if self.crop_coords:
            opts.update({
                "crop_x": self.crop_coords[0],
                "crop_y": self.crop_coords[1],
                "crop_w": self.crop_coords[2],
                "crop_h": self.crop_coords[3]
            })
        return opts

    def trigger_preview(self):
        if not self.original_image_path:
            return

        def run():
            try:
                options = self.get_options()
                # 修复: 显式指定 MIME type
                filename = os.path.basename(self.original_image_path)
                files = {'file': (filename, open(self.original_image_path, 'rb'), 'image/png')}
                data = {'options': json.dumps(options)}

                resp = requests.post(
                    f"{self.api_url}/images/preview",
                    files=files,
                    data=data,
                    headers={"Authorization": f"Bearer {self.token}"} if self.token else {}
                )

                if resp.status_code == 200:
                    img_data = resp.content
                    self.show_preview(img_data)
                else:
                    print(f"Preview failed: {resp.text}")
            except Exception as e:
                print(f"Preview error: {e}")

        threading.Thread(target=run).start()

    def show_preview(self, img_bytes):
        try:
            pil_img = Image.open(io.BytesIO(img_bytes))
            # 放大一点显示，方便看像素点
            display_img = pil_img.resize((500, 244), Image.Resampling.NEAREST)
            self.tk_preview_image = ImageTk.PhotoImage(display_img)
            self.preview_label.config(image=self.tk_preview_image, text="")
        except Exception as e:
            print(e)

    def upload_image(self):
        if not self.token:
            messagebox.showerror("错误", "请先登录")
            return

        try:
            options = self.get_options()
            # 修复: 显式指定 MIME type
            filename = os.path.basename(self.original_image_path)
            files = {'file': (filename, open(self.original_image_path, 'rb'), 'image/png')}
            data = {'options': json.dumps(options)}

            resp = requests.post(
                f"{self.api_url}/images/",
                files=files,
                data=data,
                headers={"Authorization": f"Bearer {self.token}"}
            )

            if resp.status_code == 200:
                data = resp.json()
                messagebox.showinfo("成功", f"上传成功!\nID: {data['id']}\nURL: {data['url']}")
            else:
                messagebox.showerror("失败", resp.text)
        except Exception as e:
            messagebox.showerror("错误", str(e))

if __name__ == "__main__":
    app = InfinityTagClient()
    app.mainloop()
