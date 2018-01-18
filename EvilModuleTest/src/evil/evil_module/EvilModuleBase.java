package evil.evil_module;

/**
 * Created by David Goeth on 15.01.2018.
 */
public abstract class EvilModuleBase<Type> {
  protected abstract Type doInBackgroundHook(Type... messages);
}