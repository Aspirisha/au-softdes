package au.sdshell.driver;

import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;

public class Piper implements java.lang.Runnable {

    private java.io.InputStream input;

    private java.io.OutputStream output;

    public Piper(java.io.InputStream input, java.io.OutputStream output) {
        this.input = input;
        this.output = output;
    }

    public void run() {
        try {
            // Create 512 bytes buffer
            byte[] b = new byte[512];
            int read = 1;
            // As long as data is read; -1 means EOF
            while (read > -1) {
                // Read bytes into buffer
                read = input.read(b, 0, b.length);
                //System.out.println("read: " + new String(b));
                if (read > -1) {
                    // Write bytes to output
                    output.write(b, 0, read);
                }
            }
        } catch (Exception e) {
            // Something happened while reading or writing streams; pipe is broken
            throw new RuntimeException("Broken pipe", e);
        } finally {
            try {
                input.close();
            } catch (Exception e) {
                System.out.println("Broken pipe");
                Logger.getGlobal().log(Level.INFO, e.getMessage());
            }
            try {
                output.close();
            } catch (Exception e) {
                System.out.println("Broken pipe");
                Logger.getGlobal().log(Level.INFO, e.getMessage());
            }
        }
    }

    public static java.io.InputStream pipe(List<Process> proc) throws InterruptedException {
        // Start Piper between all processes
        java.lang.Process p1;
        java.lang.Process p2;
        for (int i = 0; i < proc.size(); i++) {
            p1 = proc.get(i);
            // If there's one more process
            if (i + 1 < proc.size()) {
                p2 = proc.get(i + 1);
                // Start piper
                new Thread(new Piper(p1.getInputStream(), p2.getOutputStream())).start();
            }
        }
        java.lang.Process last = proc.get(proc.size() - 1);
        // Wait for last process in chain; may throw InterruptedException
        last.waitFor();
        // Return its InputStream
        return last.getInputStream();
    }

}