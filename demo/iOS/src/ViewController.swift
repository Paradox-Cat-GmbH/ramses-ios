//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

import UIKit
import GLKit

class ViewController: UIViewController {
    private var eaglLayer: CAEAGLLayer!
    private var rendererBundle: RendererBundle?

    override func viewDidLoad() {
        super.viewDidLoad()
        eaglLayer = CAEAGLLayer()
        view.layer.addSublayer(eaglLayer)
        eaglLayer.frame = view.frame
        
        eaglLayer.isOpaque = true
        eaglLayer.drawableProperties = [kEAGLDrawablePropertyRetainedBacking: NSNumber(booleanLiteral: false), kEAGLDrawablePropertyColorFormat: kEAGLColorFormatRGBA8]
    }


    private func recreateRenderer() {
        destroyRenderer()
        
        rendererBundle = RendererBundle(caeaglLayer: eaglLayer,
                                        width: Int32(eaglLayer.frame.size.width),
                                        height: Int32(eaglLayer.frame.size.height),
                                        interfaceSelectionIP: "127.0.0.1",
                                        daemonIP: "127.0.0.1")
        rendererBundle?.connect()
        rendererBundle?.run()
    }

    private func destroyRenderer() {
        rendererBundle = nil
    }
    
    override func viewWillTransition(to size: CGSize, with coordinator: UIViewControllerTransitionCoordinator) {
        coordinator.animate(alongsideTransition: nil) { context in
            self.eaglLayer.frame = self.view.frame
            self.recreateRenderer()
        }
    }
    
    override func viewDidAppear(_ animated: Bool) {
        recreateRenderer()
    }

    override func viewDidDisappear(_ animated: Bool) {
        destroyRenderer()
    }
}
