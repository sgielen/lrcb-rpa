using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using DirectShowLib;

namespace WebcamControl
{
    class Program
    {
        static void PrintUsage()
        {
            Console.Out.WriteLine("Usage: list | focus <name> (auto | <value:0.0-1.0>)");
        }

        static void Main(string[] args)
        {
            if (args.Length == 0)
            {
                PrintUsage();
                Environment.Exit(1);
            }

            var command = args[0];

            DsDevice[] devs = DsDevice.GetDevicesOfCat(FilterCategory.VideoInputDevice);

            if (command == "list")
            {
                for(var j = 0; j < devs.Length; ++j)
                {
                    Console.Out.WriteLine(devs[j].Name);
                }
                Environment.Exit(0);
            }

            if (args.Length != 3 || command != "focus")
            {
                PrintUsage();
                Environment.Exit(1);
            }

            DsDevice dev = null;
            for(var i = 0; i < devs.Length; ++i)
            {
                if (devs[i].Name == args[1])
                {
                    dev = devs[i];
                }
            }

            if (dev == null)
            {
                Console.Out.WriteLine("No such camera");
                Environment.Exit(1);
            }

            IFilterGraph2 graphBuilder = new FilterGraph() as IFilterGraph2;
            IBaseFilter capFilter = null;
            graphBuilder.AddSourceFilterForMoniker(dev.Mon, null, dev.Name, out capFilter);
            IAMCameraControl _camera = capFilter as IAMCameraControl;

            int pMin, pMax, pSteppingDelta, pDefault;
            CameraControlFlags pCapsFlags;
            _camera.GetRange(CameraControlProperty.Focus, out pMin, out pMax, out pSteppingDelta, out pDefault, out pCapsFlags);

            var valueStr = args[2];

            if (valueStr == "auto")
            {
                _camera.Set(CameraControlProperty.Focus, pDefault, CameraControlFlags.Auto);
                Environment.Exit(0);
            }

            var value = float.Parse(valueStr, System.Globalization.CultureInfo.InvariantCulture);

            int pSteps = (pMax - pMin) / pSteppingDelta;
            int pStep = (int)Math.Round(value * pSteps);
            int pValue = pMin + pSteppingDelta * pStep;
            _camera.Set(CameraControlProperty.Focus, pValue, CameraControlFlags.Manual);
            Environment.Exit(0);
        }
    }
}
