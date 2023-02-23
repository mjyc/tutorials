#include <rclcpp/rclcpp.hpp>
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "tf2_geometry_msgs/tf2_geometry_msgs.h"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2/exceptions.h"
#include "tf2_ros/transform_listener.h"
#include "tf2_ros/buffer.h"

class TfListener : public rclcpp::Node{
private:
    void poseCallback(const geometry_msgs::msg::PoseStamped::SharedPtr msg){
        // IMPORTANT 
        // Use "try-catch" semantics to avoid problems when a transform does not exists
        try{
            pose_in_ = *msg;
            // LookupTransform will operate all the matrices between the target_frame and the source_frame
            tf_buffer_->transform<geometry_msgs::msg::PoseStamped>(pose_in_, pose_out_, "arm_end_link", 
                tf2::Duration(std::chrono::seconds(1)));
            RCLCPP_INFO(get_logger(),"Object pose in 'arm_end_link' is:\n x,y,z = %.1f,%.1f,%.1f",
                pose_out_.pose.position.x,
                pose_out_.pose.position.y,
                pose_out_.pose.position.z);
        }
        catch (const tf2::TransformException & ex){
            RCLCPP_WARN(get_logger(),"Could not find object position in 'arm_end_link' frame.");
            return;
        }
    }
    // Subscribe to pose published by sensor node
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr pose_sub_;
    // Transform listener
    std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
    // Transforms buffer
    std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
    // Pose in destination frame
    geometry_msgs::msg::PoseStamped pose_in_;
    geometry_msgs::msg::PoseStamped pose_out_;

public:
    TfListener(const std::string & name):
    Node(name){
        // Load a buffer of transforms
        tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());
        // Make a transform listener of the buffer
        tf_listener_ = std::make_unique<tf2_ros::TransformListener>(*tf_buffer_);
        // Initialize timer that checks the transforms each second.
        pose_sub_ = create_subscription<geometry_msgs::msg::PoseStamped>
            ("/detected_object",10,
            std::bind(&TfListener::poseCallback, this, std::placeholders::_1));
    }
    ~TfListener(){}
};

int main(int argc, char const *argv[]){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<TfListener>("tf_listener");
    rclcpp::spin(node);
    return 0;
}
