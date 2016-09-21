package au.sdshell.common;

import java.io.File;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.*;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

/**
 * Created by andy on 9/16/16.
 */
public class Environment {
    private Map<String, String> variables;

    private static final Environment instance = new Environment();

    private Environment() {
        variables = new HashMap<>();
        variables.putAll(System.getenv());
    }

    public void clearVariables() {
        variables.clear();
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
        return Paths.get(variables.get("PWD"));
    }

    public void setCurrentDir(Path newDir) {
        variables.put("PWD", newDir.toString());
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
                    s = s.replaceFirst(Pattern.quote(temp), instance.getVariable(temp.substring(1)));
                    foundVariable = true;
                }
            }

            if (!foundVariable) {
                s = s.replace(s.substring(currentIndex, lastPossibleEnding), "");
            }

        }
        return s;
    }

    public final Map<String, String> getEnvironmentMap() {
        return variables;
    }
}
