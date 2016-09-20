package au.sdshell.driver;

import java.io.File;
import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.List;

/**
 * Created by andy on 9/18/16.
 */
public class ToolResolver {
    private static boolean findInFolder(Path dir, String name) {
        File folder = new File(dir.toString());
        File[] listOfFiles = folder.listFiles();

        if (listOfFiles == null)
            return false;

        for (File f : listOfFiles) {
            if (f.getName().equals(name)) {
                return true;
            }
        }
        return false;
    }

    private static Path findToolInPath(String toolName) {
        List<Path> paths = Environment.getInstance().getPathPaths();

        for (Path path : paths) {
            if (findInFolder(path, toolName))
                return path.resolve(toolName);
        }

        return null;
    }

    private static Path findToolAbsolutePath(String toolName) {
        File folder = new File(Environment.getInstance().getCurrentDir().toString());
        File[] listOfFiles = folder.listFiles();
        try {
            return Paths.get(toolName).toRealPath();
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }

    private static Path findToolRelativePath(String toolName) {
        return Environment.getInstance().getCurrentDir().resolve(toolName);
    }

    public static Path findTool(String toolName) {
        Path p = Paths.get(toolName);
        if (p.isAbsolute()) {
            return findToolAbsolutePath(toolName);
        }

        if (toolName.contains(File.separator)) {
            return findToolRelativePath(toolName);
        }

        return findToolInPath(toolName);
    }
}
