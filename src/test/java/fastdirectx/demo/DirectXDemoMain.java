package fastdirectx.demo;

import fastdirectx.DirectXBackend;
import fastgraphics.g2d.FastGraphics2D;
import fastwindow.FastNativeWindow;
import fastwindow.FastWindow;

public final class DirectXDemoMain {

    public static void main(String[] args) {
        int width = 1280;
        int height = 720;

        try (FastNativeWindow window = FastWindow.create("FastDirectX 11 — Raster Test Pattern", width, height);
             DirectXBackend backend = new DirectXBackend()) {

            long hwnd = window.getHWND();
            backend.initialize(hwnd, width, height);

            FastGraphics2D g2d = new FastGraphics2D(backend, width, height);

            final int[] dimTracker = new int[] { width, height };

            java.util.function.BiConsumer<Integer, Integer> renderFrame = (curW, curH) -> {
                if (curW <= 0 || curH <= 0) return;

                if (curW != dimTracker[0] || curH != dimTracker[1]) {
                    backend.resize(curW, curH);
                    dimTracker[0] = curW;
                    dimTracker[1] = curH;
                }

                g2d.updateDimensions(curW, curH);
                g2d.begin();

                // Dunkelgrauer Hintergrund
                g2d.clear(0.08f, 0.08f, 0.08f, 1.0f);

                // 1. Raster-Testbild: 8x8 Farb-Kacheln über obere 80% des Fensters gestreckt
                float gridH = (float) curH * 0.80f;
                int cols = 8;
                int rows = 8;
                float cellW = (float) curW / cols;
                float cellH = gridH / rows;

                for (int y = 0; y < rows; y++) {
                    for (int x = 0; x < cols; x++) {
                        float r = x / (float) (cols - 1);
                        float g = y / (float) (rows - 1);
                        float b = (x + y) / (float) ((cols - 1) + (rows - 1));

                        g2d.setColor(r, g, b, 1.0f);
                        g2d.fillRect(x * cellW, y * cellH, cellW, cellH);
                    }
                }

                // 2. Horizontales Farbspektrum (Gradient-Balken über untere 20% des Fensters gestreckt)
                int gradSteps = 256;
                float gradW = (float) curW / gradSteps;
                float gradY = gridH;
                float gradH = (float) curH - gridH;

                for (int i = 0; i < gradSteps; i++) {
                    float t = i / (float) (gradSteps - 1);
                    float r = (float) Math.sin(t * Math.PI);
                    float g = (float) Math.sin((t + 0.33f) * Math.PI);
                    float b = (float) Math.sin((t + 0.66f) * Math.PI);

                    g2d.setColor(Math.max(0, r), Math.max(0, g), Math.max(0, b), 1.0f);
                    g2d.fillRect(i * gradW, gradY, gradW + 1.0f, gradH);
                }

                g2d.end();
            };

            // Synchronous listener during Windows modal sizing loop
            window.setPaintListener(renderFrame::accept);

            // Render first frame before showing window to eliminate white flash
            renderFrame.accept(width, height);
            window.setVisible(true);

            long lastTime = System.nanoTime();
            int frames = 0;

            while (window.pollEvents()) {
                renderFrame.accept(window.getWidth(), window.getHeight());

                int curW = window.getWidth();
                int curH = window.getHeight();

                frames++;
                long now = System.nanoTime();
                if (now - lastTime >= 1_000_000_000L) {
                    window.setTitle("FastDirectX 11 — FPS: " + frames + " (" + curW + "x" + curH + ")");
                    frames = 0;
                    lastTime = now;
                }
            }
        }

        System.out.println("FastDirectX demo window closed cleanly.");
    }
}
