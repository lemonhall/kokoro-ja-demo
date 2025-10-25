plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.kotlin.android)
}

android {
    namespace = "com.lsl.kokoro_ja_android"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.lsl.kokoro_ja_android"
        minSdk = 24
        targetSdk = 34
        versionCode = 1
        versionName = "1.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        
        // 指定要打包的 native 库架构
        ndk {
            abiFilters += listOf("armeabi-v7a", "arm64-v8a", "x86", "x86_64")
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }
    kotlinOptions {
        jvmTarget = "1.8"
    }
    
    // 解决 Kuromoji 资源文件冲突
    packaging {
        resources {
            excludes += listOf(
                "META-INF/CONTRIBUTORS.md",
                "META-INF/LICENSE.md",
                "META-INF/NOTICE.md"
            )
        }
    }
}

dependencies {

    implementation(libs.androidx.core.ktx)
    implementation(libs.androidx.appcompat)
    implementation(libs.material)
    implementation(libs.androidx.activity)
    implementation(libs.androidx.constraintlayout)
    
    // ONNX Runtime for Android
    implementation(libs.onnxruntime.android)
    
    // 协程支持（用于异步推理）
    implementation(libs.kotlinx.coroutines.android)
    
    // Kuromoji 日语分词器（汉字→假名）
    implementation("com.atilika.kuromoji:kuromoji-ipadic:0.9.0")
    
    // TinyPinyin 中文拼音库（汉字→拼音）
    implementation("com.github.promeg:tinypinyin:2.0.3")
    
    testImplementation(libs.junit)
    androidTestImplementation(libs.androidx.junit)
    androidTestImplementation(libs.androidx.espresso.core)
}