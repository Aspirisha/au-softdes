package au.sdshell.common;

import java.io.File;
import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.LinkedList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Created by andy on 9/21/16.
 */
public class FileResolver {
    public static List<Path> getPathPaths() {
        String pathVariableValue = Environment.getInstance().getVariable("PATH");
        LinkedList<Path> res = new LinkedList<>();
        if (pathVariableValue == null)
            return res;

        Matcher m = Pattern.compile("([^\"][^" + File.pathSeparator + "]*|\".+?\")" +
                File.pathSeparator).matcher(pathVariableValue);

        int beginOfLastPath = 0;
        while (m.find()) {
            res.add(Paths.get(m.group(1)));
            beginOfLastPath = m.end();
        }

        if (beginOfLastPath < pathVariableValue.length()) {
            res.add(Paths.get(pathVariableValue.substring(beginOfLastPath)));
        }
        return res;
    }

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

    private static Path findFileInPath(String toolName) {
        List<Path> paths = getPathPaths();

        for (Path path : paths) {
            if (findInFolder(path, toolName))
                return path.resolve(toolName);
        }

        return null;
    }

    private static Path findFileAbsolutePath(String toolName) {
        try {
            return Paths.get(toolName).toRealPath();
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }

    private static Path findFileRelativePath(String toolName) {
        return Paths.get(System.getenv("PWD")).resolve(toolName);
    }

    public static Path findFile(String toolName) {
        Path p = Paths.get(toolName);
        if (p.isAbsolute()) {
            return findFileAbsolutePath(toolName);
        }

        if (toolName.contains(File.separator)) {
            return findFileRelativePath(toolName);
        }


        return findFileInPath(toolName);
    }
}
