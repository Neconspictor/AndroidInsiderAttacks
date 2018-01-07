package de.unipassau.fim.reallife_security.demoapp.demoapp;

import android.app.Application;
import android.content.Context;
import android.util.Log;

import java.io.File;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import dalvik.system.PathClassLoader;

/**
 * Created by Neconspictor on 30.12.2017.
 */

public class Test {

    public static Context getCurrentContext() {
        Class clazz;
        try {
            clazz = Class.forName("android.app.ActivityThread");
            Method currentActivityThreadMethod = clazz.getMethod("currentActivityThread");
            Object currentActivityThread = currentActivityThreadMethod.invoke(null);

            Method  getApplicationMethod = clazz.getMethod("getApplication");
            return (Application) getApplicationMethod.invoke(currentActivityThread);
        } catch (ClassNotFoundException | NoSuchMethodException | IllegalAccessException
                | InvocationTargetException e) {
            e.printStackTrace();
        }

        return null;
    }


    public static File getInternalStorageDirectory() {
        Context context = getCurrentContext();
        return context.getFilesDir();
    }

    public static ClassLoader createDexPathClassLoader(String dexFileName) {
        Context context = getCurrentContext();
        File internalStorage = getInternalStorageDirectory();
        String internalStoragePath = internalStorage.getAbsolutePath();
        //ClassLoader systemLoader = ClassLoader.getSystemClassLoader();

        String path = internalStoragePath + "/" +  dexFileName;
        //return new DexClassLoader(path,
        //        context.getDir("outdex", Context.MODE_PRIVATE).getAbsolutePath(),
        //        path, context.getClassLoader());

        return new PathClassLoader(path, context.getClassLoader());
    }

    public static void test() {

        ClassLoader loader = createDexPathClassLoader("EvilModule.apk");
        Class evilModuleClass = null;
        try {
            evilModuleClass = loader.loadClass("evil.evilmodule.EvilModule2");
        } catch (ClassNotFoundException e) {
            Log.e("ClassLoaderTest", "Couldn't load evil.evilmodule.EvilModule!", e);
            return;
        }

        Log.e("ClassLoaderTest", "Loaded successfully evil.evilmodule.EvilModule");

        try {
            Method foo = evilModuleClass.getMethod("foo");
            foo.invoke(null);
        } catch (NoSuchMethodException e) {
            Log.e("ClassLoaderTest", "Couldn't find foo method!", e);
        } catch (IllegalAccessException | InvocationTargetException e) {
            Log.e("ClassLoaderTest", "Couldn't invoke foo!", e);
        }
    }
}