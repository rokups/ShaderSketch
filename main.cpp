//
// Copyright (c) 2016 Rokas Kupstys
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#include <functional>

#include <Atomic/Engine/Application.h>
#include <Atomic/Core/CoreEvents.h>
#include <Atomic/Scene/Scene.h>
#include <Atomic/Graphics/Graphics.h>
#include <Atomic/Graphics/Renderer.h>
#include <Atomic/Graphics/Octree.h>
#include <Atomic/Graphics/Camera.h>
#include <Atomic/Graphics/Model.h>
#include <Atomic/Graphics/StaticModel.h>
#include <Atomic/Graphics/DebugRenderer.h>
#include <Atomic/Graphics/Technique.h>
#include <Atomic/Graphics/Texture2D.h>
#include <Atomic/IO/FileSystem.h>
#include <Atomic/IO/FileWatcher.h>
#include <Atomic/Resource/ResourceMapRouter.h>
#include <Atomic/Input/Input.h>
#include <Atomic/Graphics/CustomGeometry.h>
#include <Atomic/UI/SystemUI/SystemUI.h>
#include <Atomic/UI/SystemUI/BorderImage.h>
#include <Atomic/UI/SystemUI/SystemUIEvents.h>
#include <Atomic/Core/StringUtils.h>

using namespace Atomic;
using namespace std::placeholders;

#define logger        (*GetSubsystem<Atomic::Log>())
#define graphics      (*GetSubsystem<Atomic::Graphics>())
#define cache         (*GetSubsystem<Atomic::ResourceCache>())
#define inputsys      (*GetSubsystem<Atomic::Input>())
#define rendersys     (*GetSubsystem<Atomic::Renderer>())
#define sysui         (*GetSubsystem<Atomic::SystemUI::SystemUI>())
#define filesys       (*GetSubsystem<Atomic::FileSystem>())


class ShaderResourceRouter
    : public ResourceRouter
{
    ATOMIC_OBJECT(ShaderResourceRouter, ResourceRouter);

protected:
    String _virtual_shader_path;

public:
    String shader_path;

    ShaderResourceRouter(Context* context)
        : ResourceRouter(context)
    {
        _virtual_shader_path = graphics.GetShaderPath() + "Shader" + graphics.GetShaderExtension();
    }

    virtual void Route(String& name, StringHash type, ResourceRequest requestType)
    {
        if (shader_path.Length())
        {
            if (name == _virtual_shader_path)
                name = shader_path;
        }
    }
};

class ShaderSketch : public Application
{
ATOMIC_OBJECT(ShaderSketch, Application)
public:
    SharedPtr<Scene> _scene;
    SharedPtr<Camera> _camera;
    SharedPtr<Node> _plane;
    SharedPtr<DebugRenderer> _debug;
    SharedPtr<FileWatcher> _watcher;
    SharedPtr<ShaderResourceRouter> _router;
    Vector<SystemUI::BorderImage*> _inputs;
    SharedPtr<SystemUI::UIElement> _inputs_parent;

    ShaderSketch(Context* context) : Application(context) { }

    virtual void Setup()
    {
        // Modify engine startup parameters
        engineParameters_["WindowTitle"]   = GetTypeName();
        engineParameters_["WindowWidth"]   = 600;
        engineParameters_["WindowHeight"]  = 600;
        engineParameters_["FullScreen"]    = false;
        engineParameters_["Headless"]      = false;
        engineParameters_["Sound"]         = false;
        engineParameters_["ResourcePaths"] = "CoreData";

        context_->RegisterFactory<ShaderResourceRouter>();
    }

    virtual void Start()
    {
        _watcher = new FileWatcher(context_);

        inputsys.SetMouseVisible(true);
        _debug = new DebugRenderer(context_);

        _scene = new Scene(context_);
        _scene->CreateComponent<Octree>();

        auto camera_node = _scene->CreateChild("Camera");
        _camera = camera_node->CreateComponent<Camera>();
        _camera->SetOrthographic(true);
        _camera->SetOrthoSize(1);
        camera_node->SetPosition({0, 0, -1});
        camera_node->LookAt(Vector3::ZERO);

        cache.SetAutoReloadResources(true);
        _router = context_->CreateObject<ShaderResourceRouter>();
        cache.AddResourceRouter(_router, true);

        context_->RegisterSubsystem(new SystemUI::SystemUI(context_));
        _inputs_parent = sysui.GetRoot()->CreateChild<SystemUI::UIElement>();

        _plane = _scene->CreateChild("Plane");
        _plane->SetPosition(Vector3::ZERO);
        auto c = (float)engineParameters_["WindowWidth"].GetInt() / (float)engineParameters_["WindowHeight"].GetInt();
        auto quad = _plane->CreateComponent<CustomGeometry>();

        quad->BeginGeometry(0, TRIANGLE_LIST);
        {
            quad->DefineVertex({-1.f * c, 1.f, 0});
            quad->DefineColor({1.0, 1.0, 1.0});
            quad->DefineVertex({1.f * c, 1.f, 0});
            quad->DefineColor({1.0, 1.0, 1.0});
            quad->DefineVertex({-1.f * c, -1.f, 0});
            quad->DefineColor({1.0, 1.0, 1.0});
        }
        {
            quad->DefineVertex({-1.f * c, -1.f, 0});
            quad->DefineColor({1.0, 1.0, 1.0});
            quad->DefineVertex({1.f * c, 1.f, 0});
            quad->DefineColor({1.0, 1.0, 1.0});
            quad->DefineVertex({1.f * c, -1.f, 0});
            quad->DefineColor({1.0, 1.0, 1.0});
        }
        quad->Commit();

        rendersys.SetViewport(0, new Viewport(context_, _scene, _camera));
        SubscribeToEvent(E_UPDATE, std::bind(&ShaderSketch::OnUpdate, this, _2));
        SubscribeToEvent(E_DROPFILE, std::bind(&ShaderSketch::OnFileDrop, this, _2));
    }

    virtual void Stop()
    {
    }

    void OnUpdate(VariantMap& args)
    {
        String file_name;
        while (_watcher->GetNextChange(file_name))
            cache.ReloadResourceWithDependencies(GetPath(_router->shader_path) + file_name);

        if (inputsys.GetMouseButtonPress(MOUSEB_MIDDLE))
            RemoveTextureInput(sysui.GetElementAt(sysui.GetCursorPosition()));
        else if (inputsys.GetKeyPress(KEY_S) && inputsys.GetKeyDown(KEY_CTRL))
        {
            _inputs_parent->SetVisible(false);
            auto screenshot = context_->CreateObject<Image>();
            graphics.TakeScreenShot(screenshot);
            _inputs_parent->SetVisible(true);

            auto home = filesys.GetUserDocumentsDir();
            auto n = 1;
            String screenshot_path;
            for (;; n++)
            {
                screenshot_path = home + "/ShaderSketch_Screenshot_" + String(n) + ".png";
                if (!filesys.FileExists(screenshot_path))
                    break;
            }
            screenshot->SavePNG(screenshot_path);
        }
    }

    void OnFileDrop(VariantMap& args)
    {
        auto file_path = args[DropFile::P_FILENAME].GetString();
        auto lp = file_path.ToLower();
        if (lp.EndsWith(".glsl") || lp.EndsWith(".hlsl"))
            SetShaderFile(file_path);
        else if (lp.EndsWith(".png"))
            AddInputTexture(file_path);
    }

    void SetShaderFile(const String& file_path)
    {
        auto quad = _plane->GetComponent<CustomGeometry>();
        if (_router->shader_path != file_path)
        {
            _router->shader_path = file_path;
            _watcher->StartWatching(GetPath(file_path), true);
            quad->SetMaterial(0);
        }

        if (quad->GetMaterial() == nullptr)
        {
            auto technique = context_->CreateObject<Technique>();
            auto pass = technique->CreatePass("base");
            pass->SetPixelShader("Shader");
            pass->SetVertexShader("Shader");

            auto material = new Material(context_);
            material->SetTechnique(0, technique);

            for (auto i = 0; i < MAX_TEXTURE_UNITS; i++)
            {
                if (i < _inputs.Size())
                    material->SetTexture((TextureUnit)i, _inputs[i]->GetTexture());
                else
                    material->SetTexture((TextureUnit)i, nullptr);
            }

            quad->SetMaterial(material);
        }
    }

    void ResetShaderFile()
    {
        auto quad = _plane->GetComponent<CustomGeometry>();
        quad->SetMaterial(nullptr);
        SetShaderFile(_router->shader_path);
    }

    void AddInputTexture(const String& file_path)
    {
        int n = _inputs.Size();
        auto img = _inputs_parent->CreateChild<SystemUI::BorderImage>();
        img->SetEnabled(true);
        img->SetPosition({1 + n * 65, graphics.GetHeight() - 65});
        img->SetSize({64, 64});
        img->SetTexture(cache.GetResource<Texture2D>(file_path));
        _inputs.Push(img);
        ResetShaderFile();
    }

    void RemoveTextureInput(SystemUI::UIElement* clicked_element)
    {
        auto clicked_image = dynamic_cast<SystemUI::BorderImage*>(clicked_element);
        if (!clicked_image)
            return;

        _inputs.Remove(clicked_image);
        clicked_image->Remove();

        auto n = 0;
        for (auto img : _inputs)
            img->SetPosition({1 + n++ * 65, graphics.GetHeight() - 65});

        ResetShaderFile();
    }
};


ATOMIC_DEFINE_APPLICATION_MAIN(ShaderSketch);
