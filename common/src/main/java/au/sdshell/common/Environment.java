package au.sdshell.common;

import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

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

    private static boolean charIsPartOfName(char c) {
        return Character.isAlphabetic(c) || c == '_' || Character.isDigit(c);
    }

    public static String substituteVariables(String s) {
        boolean quoteState = false;
        boolean doubleQuoteState = false;
        boolean prevWasEscape = false;
        for (int i = 0; i < s.length(); i++) {
            switch (s.charAt(i)) {
                case '\'':
                    if (!prevWasEscape && !doubleQuoteState) {
                        quoteState = !quoteState;
                    }
                    break;
                case '"':
                    if (!prevWasEscape && !quoteState) {
                        doubleQuoteState = !doubleQuoteState;
                    }
                    break;
                case '\\':
                    prevWasEscape = true;
                    continue;
                case '$':
                    if (prevWasEscape || quoteState)
                        break;
                    int lastVarChar = i + 1;
                    if (!charIsPartOfName(s.charAt(lastVarChar)) ||
                            Character.isDigit(s.charAt(lastVarChar))) {
                        s = s.replace(s.substring(i, i + 1), "");
                        break;
                    }

                    for (lastVarChar++; lastVarChar < s.length() &&
                            charIsPartOfName(s.charAt(lastVarChar)); lastVarChar++);

                    String temp = s.substring(i, lastVarChar);
                    if (instance.variables.containsKey(temp.substring(1))) {
                        s = s.replaceFirst(Pattern.quote(temp), instance.getVariable(temp.substring(1)));
                    } else {
                        s = s.replace(s.substring(i, lastVarChar), "");
                    }
            }

            prevWasEscape = false;
        }
        return s;
    }

    public final Map<String, String> getEnvironmentMap() {
        return variables;
    }
}
