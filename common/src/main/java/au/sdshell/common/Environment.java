package au.sdshell.common;

import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

/**
 * Represents current shell environment: environment variables, current directory path.
 * Can perform environment variables substitution into passed string.
 * Created by andy on 9/16/16.
 */
public class Environment {
    private Map<String, String> variables;
    private static final Environment instance = new Environment();

    private Environment() {
        variables = new HashMap<>();
        variables.putAll(System.getenv());
    }

    /**
     * @return singleton instance of environment
     */
    public static Environment getInstance() {
        return instance;
    }

    /**
     * Sets value of variable
     * @param name name of variable
     * @param value desired variable value
     */
    public void setVariable(String name, String value) {
        variables.put(name, value);
    }

    /**
     * @param name environment variable name
     * @return value of variable or empty string, if variable is not set
     */
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

    /**
     * Looks for environment variables entries, prepended with $,
     * in passed string, and substitutes them unless
     * their occurrence is not inside single quotes or $ is escaped.
     * @param s string to process
     * @return string s with substituted env variables
     */
    public static String substituteVariables(String s) {
        boolean quoteState = false;
        boolean doubleQuoteState = false;
        boolean prevWasEscape = false;
        for (int i = 0; i < s.length(); i++) {
            switch (s.charAt(i)) {
                case '\'': {
                    if (!prevWasEscape && !doubleQuoteState) {
                        quoteState = !quoteState;
                    }
                    break;
                }
                case '"': {
                    if (!prevWasEscape && !quoteState) {
                        doubleQuoteState = !doubleQuoteState;
                    }
                    break;
                }
                case '\\': {
                    prevWasEscape = true;
                    continue;
                }
                case '$': {
                    if (prevWasEscape || quoteState)
                        break;
                    int lastVarChar = i + 1;
                    if (!charIsPartOfName(s.charAt(lastVarChar)) ||
                            Character.isDigit(s.charAt(lastVarChar))) {
                        s = s.replace(s.substring(i, i + 1), "");
                        break;
                    }

                    for (lastVarChar++; lastVarChar < s.length() &&
                            charIsPartOfName(s.charAt(lastVarChar)); lastVarChar++) {
                    }

                    String temp = s.substring(i, lastVarChar);
                    if (instance.variables.containsKey(temp.substring(1))) {
                        s = s.replaceFirst(Pattern.quote(temp), instance.getVariable(temp.substring(1)));
                    } else {
                        s = s.replace(s.substring(i, lastVarChar), "");
                    }
                }
            }

            prevWasEscape = false;
        }
        return s;
    }

    /**
     *
     * @return map of environment variables: key is variable name, value is variable value
     */
    public final Map<String, String> getEnvironmentMap() {
        return variables;
    }
}
