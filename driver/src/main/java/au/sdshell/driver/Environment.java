package au.sdshell.driver;

import org.apache.tools.ant.types.Commandline;
import sun.rmi.runtime.Log;

import java.io.File;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import static jdk.nashorn.internal.objects.NativeString.indexOf;

/**
 * Created by andy on 9/16/16.
 */
public class Environment {
    private Map<String, String> variables;
    private Path currentDir;

    public static final String defaultPath =
            System.getProperty("user.home") + File.separator + "AU/3_sem/software_design/sdapps" +
                    File.pathSeparator + System.getProperty("user.home") + File.separator + "/AU";
    private static final Environment instance = new Environment();

    private Environment() {
        variables = new HashMap<>();
        variables.put("PATH", defaultPath);
        currentDir = Paths.get(System.getProperty("user.dir"));
        System.out.println(currentDir);
    }

    public static Environment getInstance() {
        return instance;
    }

    public void setVariable(String name, String value) {
        variables.put(name, value);
    }

    public String getVariable(String name) {
        return variables.getOrDefault(name, "");
    }

    public Path getCurrentDir() {
        return currentDir;
    }

    public void setCurrentDir(Path newDir) {
        currentDir = newDir;
    }

    public List<Path> getPathPaths() {
        String path = getVariable("PATH");
        Matcher m = Pattern.compile("([^\"][^" + File.pathSeparator + "]*|\".+?\")" + File.pathSeparator).matcher(path);
        LinkedList<Path> res = new LinkedList<>();
        int beginOfLastPath = 0;
        while (m.find()) {
            res.add(Paths.get(m.group(1)));
            beginOfLastPath = m.end();
        }

        if (beginOfLastPath < path.length()) {
            res.add(Paths.get(path.substring(beginOfLastPath)));
        }
        return res;
    }

    public static String substituteVariables(String s) {
        for (int currentIndex = s.indexOf("$"); currentIndex != -1; currentIndex = s.indexOf("$")) {
            boolean foundVariable = false;
            int lastPossibleEnding = s.indexOf("$", currentIndex + 1);
            if (lastPossibleEnding == -1)
                lastPossibleEnding = s.length();

            for (int i = lastPossibleEnding; i > currentIndex; i--) {
                String temp = s.substring(currentIndex, i);
                if (instance.variables.containsKey(temp.substring(1))) {
                    s = s.replaceFirst(temp, instance.getVariable(temp.substring(1)));
                    foundVariable = true;
                }
            }

            if (!foundVariable) {
                s = s.replace(s.substring(currentIndex, lastPossibleEnding), "");
            }

        }
        return s;
    }
}
